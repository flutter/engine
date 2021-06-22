// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/syslog/cpp/log_settings.h>
#include <lib/syslog/cpp/macros.h>
#include <lib/syslog/global.h>
#include <lib/syslog/logger.h>
#include <lib/syslog/wire_format.h>
#include <lib/zx/socket.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/lib/fxl/command_line.h"
#include "src/lib/fxl/log_settings_command_line.h"
#include "zircon/system/ulib/syslog/include/lib/syslog/logger.h"

namespace syslog {
namespace {

struct LogPacket {
  fx_log_metadata_t metadata;
  std::vector<std::string> tags;
  std::string message;
};

class LoggingBaseSocketTest : public ::testing::Test {
 protected:
  LogPacket ReadPacket() {
    LogPacket result;
    fx_log_packet_t packet;
    socket_.read(0, &packet, sizeof(packet), nullptr);
    result.metadata = packet.metadata;
    int pos = 0;
    while (packet.data[pos]) {
      int tag_len = packet.data[pos++];
      result.tags.emplace_back(packet.data + pos, tag_len);
      pos += tag_len;
    }
    result.message.append(packet.data + pos + 1);
    return result;
  }

  void ReadPacketAndCompare(fx_log_severity_t severity, const std::string& message,
                            const std::vector<std::string>& tags = {}) {
    LogPacket packet = ReadPacket();
    EXPECT_EQ(severity, packet.metadata.severity);
    EXPECT_THAT(packet.message, testing::EndsWith(message));
    EXPECT_EQ(tags, packet.tags);
  }

  void CheckSocketEmpty() {
    zx_info_socket_t info = {};
    zx_status_t status = socket_.get_info(ZX_INFO_SOCKET, &info, sizeof(info), nullptr, nullptr);
    ASSERT_EQ(ZX_OK, status);
    EXPECT_EQ(0u, info.rx_buf_available);
  }

  zx::socket socket_;
};

class LoggingSocketTest : public LoggingBaseSocketTest {
 protected:
  void SetUp() override {
    SetLogSettings(LogSettings());

    zx::socket local;
    ASSERT_EQ(ZX_OK, zx::socket::create(ZX_SOCKET_DATAGRAM, &local, &socket_));

    fx_logger_config_t config = {.min_severity = FX_LOG_INFO,
                                 .console_fd = -1,
                                 .log_service_channel = local.release(),
                                 .tags = nullptr,
                                 .num_tags = 0};

    fx_log_reconfigure(&config);
  }
};

TEST_F(LoggingBaseSocketTest, LoggerCreatedNotConnected) {
  fx_logger_t* logger = NULL;
  fx_logger_config_t config = {.min_severity = FX_LOG_INFO,
                               .console_fd = -1,
                               .log_service_channel = ZX_HANDLE_INVALID,
                               .tags = nullptr,
                               .num_tags = 0};

  fx_logger_create_internal(&config, &logger, false);

  // |logger| should not have socket connection
  ASSERT_EQ(fx_logger_get_connection_status(logger), ZX_ERR_NOT_CONNECTED);

  fx_logger_destroy(logger);
}

TEST_F(LoggingBaseSocketTest, LoggerCreatedConnected) {
  fx_logger_t* logger = NULL;
  fx_logger_config_t config = {.min_severity = FX_LOG_INFO,
                               .console_fd = -1,
                               .log_service_channel = ZX_HANDLE_INVALID,
                               .tags = nullptr,
                               .num_tags = 0};

  fx_logger_create_internal(&config, &logger, true);

  // |logger| should be connected with socket
  ASSERT_EQ(fx_logger_get_connection_status(logger), ZX_OK);

  fx_logger_destroy(logger);
}

TEST_F(LoggingBaseSocketTest, LoggerReconfiguredConnected) {
  fx_logger_t* logger = NULL;
  fx_logger_config_t config = {.min_severity = FX_LOG_INFO,
                               .console_fd = -1,
                               .log_service_channel = ZX_HANDLE_INVALID,
                               .tags = nullptr,
                               .num_tags = 0};

  fx_logger_create_internal(&config, &logger, false);

  // |logger| should not have socket connection
  ASSERT_EQ(fx_logger_get_connection_status(logger), ZX_ERR_NOT_CONNECTED);

  zx::socket local;
  ASSERT_EQ(ZX_OK, zx::socket::create(ZX_SOCKET_DATAGRAM, &local, &socket_));

  config = {.min_severity = FX_LOG_INFO,
            .console_fd = -1,
            .log_service_channel = local.release(),
            .tags = nullptr,
            .num_tags = 0};

  fx_logger_reconfigure(logger, &config);

  // |logger| should be connected with socket
  ASSERT_EQ(fx_logger_get_connection_status(logger), ZX_OK);

  fx_logger_destroy(logger);
}

TEST_F(LoggingBaseSocketTest, LoggerSetConnected) {
  fx_logger_t* logger = NULL;
  fx_logger_config_t config = {.min_severity = FX_LOG_INFO,
                               .console_fd = -1,
                               .log_service_channel = ZX_HANDLE_INVALID,
                               .tags = nullptr,
                               .num_tags = 0};

  fx_logger_create_internal(&config, &logger, false);

  // |logger| should not have socket connection
  ASSERT_EQ(fx_logger_get_connection_status(logger), ZX_ERR_NOT_CONNECTED);

  zx::socket local;
  ASSERT_EQ(ZX_OK, zx::socket::create(ZX_SOCKET_DATAGRAM, &local, &socket_));

  fx_logger_set_connection(logger, local.release());

  // |logger| should be connected with socket
  ASSERT_EQ(fx_logger_get_connection_status(logger), ZX_OK);

  fx_logger_destroy(logger);
}

TEST_F(LoggingSocketTest, LogSimple) {
  const char* msg = "test message";
  FX_LOGS(INFO) << msg;
  ReadPacketAndCompare(FX_LOG_INFO, msg);
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, LogWithTag) {
  const char* kMsg = "just some string";
  const char* kTag = "tag";
  FX_LOGST(INFO, kTag) << "just some string";
  ReadPacketAndCompare(FX_LOG_INFO, kMsg, {kTag});
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, Check) {
  FX_CHECK(1 > 0) << "error msg";
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, VLog) {
  const char* kMsg1 = "test message";
  const char* kMsg2 = "another message";
  const char* kMsg3 = "yet another message";
  const char* kMsg4 = "last message";

  FX_VLOGS(1) << kMsg1;
  CheckSocketEmpty();

  SetLogSettings({.min_log_level = (LOG_INFO - 1), .log_file = ""}, {});
  FX_VLOGS(1) << kMsg2;
  ReadPacketAndCompare((LOG_INFO - 1), kMsg2);
  CheckSocketEmpty();

  FX_VLOGS(2) << kMsg3;
  CheckSocketEmpty();

  FX_LOGS(WARNING) << kMsg4;
  ReadPacketAndCompare(FX_LOG_WARNING, kMsg4);
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, VLogWithTag) {
  const char* kMsg1 = "test message";
  const char* kMsg2 = "another message";
  const char* kTag1 = "TAG";
  const char* kTag2 = "TAAAG";

  FX_VLOGST(1, kTag1) << kMsg1;
  CheckSocketEmpty();

  SetLogSettings({.min_log_level = (LOG_INFO - 1)}, {});
  FX_VLOGST(1, kTag2) << kMsg2;
  ReadPacketAndCompare((LOG_INFO - 1), kMsg2, {kTag2});
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, PLog) {
  FX_PLOGS(ERROR, ZX_OK) << "should be ok";
  ReadPacketAndCompare(FX_LOG_ERROR, "should be ok: 0 (ZX_OK)");
  CheckSocketEmpty();

  FX_PLOGS(INFO, ZX_ERR_ACCESS_DENIED) << "something that failed";
  ReadPacketAndCompare(FX_LOG_INFO, "something that failed: -30 (ZX_ERR_ACCESS_DENIED)");
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, PLogWithTag) {
  FX_PLOGST(WARNING, "test", ZX_ERR_IO_NOT_PRESENT) << "something bad happened";
  ReadPacketAndCompare(FX_LOG_WARNING, "something bad happened: -44 (ZX_ERR_IO_NOT_PRESENT)",
                       {"test"});
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, LogFirstN) {
  constexpr int kLimit = 5;
  constexpr int kCycles = 20;
  constexpr const char* kLogMessage = "Hello";
  static_assert(kCycles > kLimit);

  for (int i = 0; i < kCycles; ++i) {
    FX_LOGS_FIRST_N(ERROR, kLimit) << kLogMessage;
  }
  for (int i = 0; i < kLimit; ++i) {
    ReadPacketAndCompare(FX_LOG_ERROR, kLogMessage);
  }
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, LogFirstNWithTag) {
  constexpr int kLimit = 5;
  constexpr int kCycles = 20;
  constexpr const char* kLogMessage = "Hello";
  constexpr const char* kTag = "tag";
  static_assert(kCycles > kLimit);

  for (int i = 0; i < kCycles; ++i) {
    FX_LOGST_FIRST_N(ERROR, kLimit, kTag) << kLogMessage;
  }
  for (int i = 0; i < kLimit; ++i) {
    ReadPacketAndCompare(FX_LOG_ERROR, kLogMessage, {kTag});
  }
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, DontWriteSeverity) {
  FX_LOGS(ERROR) << "Hi";
  LogPacket packet = ReadPacket();
  ASSERT_THAT(packet.message, testing::Not(testing::HasSubstr("ERROR")));
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, SetSettingsAndTags) {
  constexpr const char* kLogMessage1 = "Hello";
  constexpr const char* kLogMessage2 = "Message";
  constexpr const char* kGlobalTag = "1234";
  constexpr const char* kTag = "tag";

  SetLogSettings(LogSettings(), {kGlobalTag});

  FX_LOGS(ERROR) << kLogMessage1;
  ReadPacketAndCompare(FX_LOG_ERROR, kLogMessage1, {kGlobalTag});
  CheckSocketEmpty();

  FX_LOGST(WARNING, kTag) << kLogMessage2;
  ReadPacketAndCompare(FX_LOG_WARNING, kLogMessage2, {kGlobalTag, kTag});
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, SetSettingsAndTagsFromCommandLine) {
  constexpr const char* kLogMessage = "Hello";
  constexpr const char* kTag = "1234";

  fxl::CommandLine command_line = fxl::CommandLineFromInitializerList({"argv0", "--quiet"});
  fxl::SetLogSettingsFromCommandLine(command_line, {kTag});

  FX_LOGS(ERROR) << kLogMessage;
  ReadPacketAndCompare(FX_LOG_ERROR, kLogMessage, {kTag});
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, TooManyTags) {
  constexpr const char* kLogMessage = "Hello";
  syslog::SetTags({"1", "2", "3", "4", "5"});
  FX_LOGS(ERROR) << kLogMessage;
  ReadPacketAndCompare(FX_LOG_ERROR, kLogMessage, {"1", "2", "3", "4"});
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, Structured) {
  FX_SLOG(INFO, "msg", KV("one", 1), KV("three", 3));
  ReadPacketAndCompare(FX_LOG_INFO, "msg one=1 three=3");
  CheckSocketEmpty();
}

TEST_F(LoggingSocketTest, LogId) {
  FX_LOGS(ERROR("test")) << "Hello";
  ReadPacketAndCompare(FX_LOG_ERROR, "Hello log_id=\"test\"");
  CheckSocketEmpty();
}

}  // namespace
}  // namespace syslog
