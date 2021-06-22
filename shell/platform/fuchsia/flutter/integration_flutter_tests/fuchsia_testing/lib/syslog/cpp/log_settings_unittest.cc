// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/syslog/cpp/log_settings.h>
#include <lib/syslog/cpp/macros.h>
#include <unistd.h>

#include <fbl/unique_fd.h>
#include <gtest/gtest.h>

#include "src/lib/files/file.h"
#include "src/lib/files/scoped_temp_dir.h"

namespace syslog {
namespace {

class LogSettingsFixture : public ::testing::Test {
 public:
  LogSettingsFixture() : old_severity_(GetMinLogLevel()), old_stderr_(dup(STDERR_FILENO)) {}
  ~LogSettingsFixture() {
    SetLogSettings({.min_log_level = old_severity_});
    dup2(old_stderr_.get(), STDERR_FILENO);
  }

 private:
  LogSeverity old_severity_;
  fbl::unique_fd old_stderr_;
};

TEST(LogSettings, DefaultOptions) {
  LogSettings settings;
  EXPECT_EQ(LOG_INFO, settings.min_log_level);
  EXPECT_EQ(std::string(), settings.log_file);
}

TEST_F(LogSettingsFixture, SetAndGet) {
  LogSettings new_settings;
  new_settings.min_log_level = -20;
  SetLogSettings(new_settings);
  EXPECT_EQ(new_settings.min_log_level, GetMinLogLevel());
}

TEST_F(LogSettingsFixture, SetValidLogFile) {
  const char kTestMessage[] = "TEST MESSAGE";

  LogSettings new_settings;
  files::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.NewTempFile(&new_settings.log_file));
  SetLogSettings(new_settings);

  FX_LOGS(INFO) << kTestMessage;

  ASSERT_EQ(0, access(new_settings.log_file.c_str(), R_OK));
  std::string log;
  ASSERT_TRUE(files::ReadFileToString(new_settings.log_file, &log));
  EXPECT_TRUE(log.find(kTestMessage) != std::string::npos);
}

TEST_F(LogSettingsFixture, SetInvalidLogFile) {
  LogSettings new_settings;
  new_settings.log_file = "\\\\//invalid-path";
  SetLogSettings(new_settings);

  EXPECT_NE(0, access(new_settings.log_file.c_str(), R_OK));
}

}  // namespace
}  // namespace syslog
