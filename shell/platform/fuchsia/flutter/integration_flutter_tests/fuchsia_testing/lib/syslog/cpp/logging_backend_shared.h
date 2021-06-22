// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_SYSLOG_CPP_LOGGING_BACKEND_SHARED_H_
#define LIB_SYSLOG_CPP_LOGGING_BACKEND_SHARED_H_

#include <assert.h>
#include <lib/syslog/cpp/log_level.h>
#include <lib/syslog/cpp/macros.h>

#include <cstring>

// This file contains shared implementations for writing string logs between the legacy backend and
// the host backend

namespace syslog_backend {
struct MsgHeader {
  syslog::LogSeverity severity;
  char* offset;
  LogBuffer* buffer;
  bool first_tag;
  char* user_tag;
  bool has_msg;
  bool first_kv;
  bool terminated;
  size_t RemainingSpace() {
    auto end = (reinterpret_cast<const char*>(this) + sizeof(LogBuffer));
    auto avail = static_cast<size_t>(end - offset -
                                     2);  // Allocate an extra byte for a NULL terminator at the end
    return avail;
  }

  void NullTerminate() {
    if (!terminated) {
      terminated = true;
      *(offset++) = 0;
    }
  }

  const char* c_str() {
    NullTerminate();
    return reinterpret_cast<const char*>(buffer->data);
  }

  void WriteChar(const char value) {
    if (!RemainingSpace()) {
      FlushRecord(buffer);
      offset = reinterpret_cast<char*>(buffer->data);
      WriteString("CONTINUATION: ");
    }
    assert((offset + 1) < (reinterpret_cast<const char*>(this) + sizeof(LogBuffer)));
    *offset = value;
    offset++;
  }

  void WriteString(const char* c_str) {
    size_t total_chars = strlen(c_str);
    size_t written_chars = 0;
    while (written_chars < total_chars) {
      size_t written = WriteStringInternal(c_str + written_chars);
      written_chars += written;
      if (written_chars < total_chars) {
        FlushAndReset();
        WriteStringInternal("CONTINUATION: ");
      }
    }
  }

  void FlushAndReset() {
    FlushRecord(buffer);
    offset = reinterpret_cast<char*>(buffer->data);
  }

  // Writes a string to the buffer and returns the
  // number of bytes written. Returns 0 only if
  // the length of the string is 0, or if we're
  // exactly at the end of the buffer and need a reset.
  size_t WriteStringInternal(const char* c_str) {
    size_t len = strlen(c_str);
    auto remaining = RemainingSpace();
    if (len > remaining) {
      len = remaining;
    }
    assert((offset + len) < (reinterpret_cast<const char*>(this) + sizeof(LogBuffer)));
    memcpy(offset, c_str, len);
    offset += len;
    return len;
  }

  void Init(LogBuffer* buffer, syslog::LogSeverity severity) {
    this->severity = severity;
    user_tag = nullptr;
    offset = reinterpret_cast<char*>(buffer->data);
    first_tag = true;
    has_msg = false;
    first_kv = true;
    terminated = false;
  }

  static MsgHeader* CreatePtr(LogBuffer* buffer) {
    return reinterpret_cast<MsgHeader*>(&buffer->record_state);
  }
};

#ifndef __Fuchsia__
const std::string GetNameForLogSeverity(syslog::LogSeverity severity);
#endif

static_assert(sizeof(MsgHeader) <= sizeof(LogBuffer::record_state),
              "message header must be no larger than record_state");

void BeginRecordLegacy(LogBuffer* buffer, syslog::LogSeverity severity, const char* file,
                       unsigned int line, const char* msg, const char* condition);

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, const char* value);

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, int64_t value);

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, uint64_t value);

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, double value);

void EndRecordLegacy(LogBuffer* buffer);

}  // namespace syslog_backend

#endif  // LIB_SYSLOG_CPP_LOGGING_BACKEND_SHARED_H_
