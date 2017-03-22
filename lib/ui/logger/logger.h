// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_LOGGER_LOGGER_H_
#define FLUTTER_LIB_UI_LOGGER_LOGGER_H_

#include <cstddef>
#include <memory>
#include <thread>
#include "lib/ftl/files/unique_fd.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/synchronization/mutex.h"

namespace blink {

class Logger {
 public:
  static const size_t kDisabledPort = 0;

  static void InitializeLogger(size_t port);

  static Logger& GetLogger();

  void Log(const uint8_t* message, size_t length);

 private:
  ftl::UniqueFD server_socket_;
  ftl::Mutex client_socket_mutex_;
  ftl::UniqueFD client_socket_ FTL_GUARDED_BY(client_socket_mutex_);
  std::unique_ptr<std::thread> listener_thread_;

  Logger(size_t port);

  ~Logger();

  bool LogSingle(const uint8_t* message, size_t length);

  void ListenForClients();

  FTL_DISALLOW_COPY_AND_ASSIGN(Logger);
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_LOGGER_LOGGER_H_
