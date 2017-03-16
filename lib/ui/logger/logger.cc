// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/logger/logger.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "flutter/lib/ui/logger/system_logger.h"
#include "lib/ftl/files/eintr_wrapper.h"

namespace blink {

static Logger* g_logger = nullptr;

void Logger::InitializeLogger(size_t port) {
  FTL_DCHECK(g_logger == nullptr);
  g_logger = new Logger(port);
}

Logger& Logger::GetLogger() {
  FTL_DCHECK(g_logger != nullptr)
      << "Logger must be initialized via a call to initialize logger.";
  return *g_logger;
}

static bool ConfigureSocketBuffer(const ftl::UniqueFD& handle) {
  // Update the send buffer size.
  const int size = 4096;
  if (::setsockopt(handle.get(), SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) !=
      0) {
    FTL_DLOG(INFO) << "Logger: Could not update socket buffer size.";
    return false;
  }

  return true;
}

static bool ConfigureClientSocket(const ftl::UniqueFD& handle) {
  if (!ConfigureSocketBuffer(handle)) {
    return false;
  }

  // Make the socket non-blocking.
  if (HANDLE_EINTR(::fcntl(handle.get(), F_SETFL, O_NONBLOCK)) == -1) {
    FTL_DLOG(INFO) << "Logger: Could not make socket non-blocking.";
    return false;
  }

  return true;
}

static ftl::UniqueFD CreateServerSocket(size_t port) {
  if (port == Logger::kDisabledPort) {
    return {};
  }

  ftl::UniqueFD handle(::socket(AF_INET, SOCK_STREAM, 0));

  if (!handle.is_valid()) {
    return {};
  }

  if (!ConfigureSocketBuffer(handle)) {
    return {};
  }

  // Bind the socket to the port.
  struct sockaddr_in local = {};
  local.sin_len = sizeof(local);
  local.sin_family = AF_INET;
  local.sin_port = htons(port);
  local.sin_addr.s_addr = INADDR_ANY;

  auto len = sizeof(local);
  auto local_address = reinterpret_cast<struct sockaddr*>(&local);

  if (::bind(handle.get(), local_address, len) != 0) {
    FTL_DLOG(INFO) << "Logger: Could not bind socket to port " << port
                   << ". Error: " << strerror(errno) << " (" << errno << ").";
    return {};
  }

  // Listen on the port.
  if (::listen(handle.get(), 1) != 0) {
    FTL_DLOG(INFO) << "Logger: Could not listen on the clients.";
    return {};
  }

  return handle;
}

Logger::Logger(size_t port) : server_socket_(CreateServerSocket(port)) {
  if (server_socket_.is_valid()) {
    listener_thread_ =
        std::make_unique<std::thread>([this]() { ListenForClients(); });
  }
}

Logger::~Logger() {
  server_socket_.reset();
  if (listener_thread_) {
    listener_thread_->join();
  }
}

void Logger::Log(const uint8_t* allocation, size_t allocation_size) {
  // Attempt logging to a connected client.
  if (LogSingle(allocation, allocation_size)) {
    return;
  }

  // Attempt logging to the system logger.
  SytemLoggerLog(allocation, allocation_size);
}

bool Logger::LogSingle(const uint8_t* allocation, size_t allocation_size) {
  ftl::MutexLocker lock(&client_socket_mutex_);
  if (!client_socket_.is_valid()) {
    return false;
  }
  size_t sent = 0;
  while (sent < allocation_size) {
    int64_t sent_this_time = HANDLE_EINTR(::send(
        client_socket_.get(), allocation + sent, allocation_size - sent, 0));
    if (sent_this_time >= 0) {
      sent += sent_this_time;
    } else {
      // This includes an EAGAIN in case the socket is full.
      break;
    }
  }
  return sent == allocation_size;
}

void Logger::ListenForClients() {
  while (true) {
    struct sockaddr_in client = {};

    auto clientAddress = reinterpret_cast<struct sockaddr*>(&client);
    auto length = static_cast<socklen_t>(sizeof(client));

    ftl::UniqueFD accepted(
        HANDLE_EINTR(::accept(server_socket_.get(), clientAddress, &length)));

    if (!accepted.is_valid()) {
      return;
    }

    if (!ConfigureClientSocket(accepted)) {
      FTL_DLOG(INFO)
          << "Logger: Could not configure accepted socket as client.";
      continue;
    }

    ftl::MutexLocker lock(&client_socket_mutex_);
    client_socket_.swap(accepted);
    FTL_DLOG(INFO) << "Logger: Found client.";
  }
  FTL_DLOG(INFO) << "Logger: Terminating listen.";
}

}  // namespace blink
