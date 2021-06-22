// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/syslog/cpp/log_settings.h>
#include <lib/syslog/cpp/logging_backend.h>
#include <lib/syslog/cpp/logging_backend_fuchsia_private.h>
#include <lib/syslog/cpp/macros.h>

#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "logging_backend_shared.h"
#include "src/lib/files/file.h"
#include "src/lib/files/scoped_temp_dir.h"

namespace syslog {

TEST(StructuredLogging, Log) {
  FX_SLOG(WARNING, "test_log", KV("foo", "bar"));
  // TODO(fxbug.dev/57482): Figure out how to verify this appropriately.
}

TEST(StructuredLogging, BackendDirect) {
  syslog_backend::LogBuffer buffer;
  syslog_backend::BeginRecord(&buffer, syslog::LOG_WARNING, "foo.cc", 42, "fake tag", "condition");
  syslog_backend::EndRecord(&buffer);
  syslog_backend::FlushRecord(&buffer);
  syslog_backend::BeginRecord(&buffer, syslog::LOG_WARNING, "foo.cc", 42, "fake tag", "condition");
  syslog_backend::WriteKeyValue(&buffer, "foo", static_cast<int64_t>(42));
  syslog_backend::EndRecord(&buffer);
  ASSERT_TRUE(syslog_backend::FlushRecord(&buffer));
  // TODO(fxbug.dev/57482): Figure out how to verify this appropriately.
}

TEST(StructuredLogging, StartsAtPosition0) {
  syslog_backend::LogBuffer buffer;
  memset(&buffer, 0, sizeof(buffer));
  syslog_backend::BeginRecord(&buffer, syslog::LOG_WARNING, "foo.cc", 42, "fake tag", "condition");
  syslog_backend::EndRecord(&buffer);
  ASSERT_NE(buffer.data[0], static_cast<uint64_t>(0));
}

TEST(StructuredLogging, PaddedWritePadsWithZeroes) {
  uint64_t fives;
  memset(&fives, 5, sizeof(fives));
  uint64_t buffer[2];
  memset(buffer, 5, sizeof(buffer));
  // Writing "hi" results in 26984 which is
  // 'h' written to byte 0 | 'i' written to byte 1
  // Bytes get reversed due to endianness so they are flipped
  // when combined.
  WritePaddedInternal(buffer, "hi", ByteOffset::Unbounded(2));
  ASSERT_EQ(buffer[0], static_cast<uint64_t>(26984));
  ASSERT_EQ(buffer[1], fives);
  buffer[0] = fives;
  buffer[1] = fives;
  WritePaddedInternal(buffer, "", ByteOffset::Unbounded(0));
  ASSERT_EQ(buffer[0], fives);
  ASSERT_EQ(buffer[1], fives);
}

TEST(ByteOffset, FromBuffer) {
  ByteOffset offset = ByteOffset::FromBuffer(5, 10);
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(5));
  ASSERT_EQ(offset.capacity(), static_cast<size_t>(10));
}

TEST(ByteOffset, Unbounded) {
  ByteOffset offset = ByteOffset::Unbounded(13);
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(13));
  ASSERT_EQ(offset.capacity(), static_cast<size_t>(-1));
}

TEST(ByteOffset, Addition) {
  ByteOffset offset = ByteOffset::Unbounded(13);
  offset = offset + 1;
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(14));
}

TEST(WordOffset, FromByteOffset) {
  WordOffset<uint64_t> offset =
      WordOffset<uint64_t>::FromByteOffset(ByteOffset::FromBuffer(8, 256));
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(1));
  ASSERT_EQ(offset.capacity(), static_cast<size_t>(32));
}

TEST(WordOffset, ToByteOffset) {
  WordOffset<uint64_t> offset =
      WordOffset<uint64_t>::FromByteOffset(ByteOffset::FromBuffer(8, 256));
  ASSERT_EQ(offset.ToByteOffset().unsafe_get(), static_cast<size_t>(8));
  ASSERT_EQ(offset.ToByteOffset().capacity(), static_cast<size_t>(256));
}

TEST(WordOffset, Addition) {
  WordOffset<uint64_t> offset =
      WordOffset<uint64_t>::FromByteOffset(ByteOffset::FromBuffer(8, 256));
  offset = offset + 1;
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(2));
  offset = offset + WordOffset<uint64_t>::FromByteOffset(ByteOffset::Unbounded(16));
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(4));
}

TEST(WordOffset, Begin) {
  WordOffset<uint64_t> offset =
      WordOffset<uint64_t>::FromByteOffset(ByteOffset::FromBuffer(8, 256));
  offset = offset.begin();
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(0));
  ASSERT_EQ(offset.capacity(), static_cast<size_t>(32));
}

TEST(WordOffset, Reset) {
  WordOffset<uint64_t> offset =
      WordOffset<uint64_t>::FromByteOffset(ByteOffset::FromBuffer(8, 256));
  offset.reset();
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(0));
  ASSERT_EQ(offset.capacity(), static_cast<size_t>(32));
}

TEST(WordOffset, InBounds) {
  WordOffset<uint64_t> offset = WordOffset<uint64_t>::FromByteOffset(ByteOffset::FromBuffer(8, 16));
  ASSERT_TRUE(offset.in_bounds(WordOffset<uint64_t>::FromByteOffset(ByteOffset::Unbounded(0))));
  ASSERT_FALSE(offset.in_bounds(WordOffset<uint64_t>::FromByteOffset(ByteOffset::Unbounded(8))));
}

TEST(WordOffset, Invalid) {
  WordOffset<uint64_t> offset = WordOffset<uint64_t>::invalid();
  ASSERT_EQ(offset.capacity(), static_cast<size_t>(0));
  ASSERT_EQ(offset.unsafe_get(), static_cast<size_t>(0));
}

TEST(StructuredLogging, Overflow) {
  std::vector<char> very_large_string;
  very_large_string.resize(1000 * 1000);
  memset(very_large_string.data(), 5, very_large_string.size());
  very_large_string[very_large_string.size() - 1] = 0;
  syslog_backend::LogBuffer buffer;
  syslog_backend::BeginRecord(&buffer, syslog::LOG_WARNING, "foo.cc", 42, "fake tag", "condition");
  syslog_backend::EndRecord(&buffer);
  syslog_backend::FlushRecord(&buffer);
  syslog_backend::BeginRecord(&buffer, syslog::LOG_WARNING, "foo.cc", 42, "fake tag", "condition");
  syslog_backend::WriteKeyValue(&buffer, "foo", static_cast<int64_t>(42));
  syslog_backend::WriteKeyValue(&buffer, "bar", very_large_string.data());

  syslog_backend::EndRecord(&buffer);
  ASSERT_FALSE(syslog_backend::FlushRecord(&buffer));
}

TEST(StructuredLogging, LOGS) {
  std::string str;
  // 5mb log shouldn't crash
  str.resize(1000 * 5000);
  memset(str.data(), 's', str.size() - 1);
  FX_LOGS(INFO) << str;
}

TEST(StructuredLogging, Legacy) {
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

TEST(StructuredLogging, ToCStr) {
  syslog_backend::LogBuffer buffer;
  memset(&buffer, 0, sizeof(buffer));
  syslog_backend::BeginRecord(&buffer, syslog::LOG_WARNING, "foo.cc", 42, "fake tag", "condition");
  syslog_backend::EndRecord(&buffer);
  auto header = syslog_backend::MsgHeader::CreatePtr(&buffer);
  // Ensure that null termination only happens once
  header->c_str();
  header->c_str();
  header->WriteString("something after the null terminator");
  ASSERT_TRUE(std::string(header->c_str()).find("fake tag") != std::string::npos);
  ASSERT_TRUE(std::string(header->c_str()).find("something after the null terminator") ==
              std::string::npos);
}

}  // namespace syslog
