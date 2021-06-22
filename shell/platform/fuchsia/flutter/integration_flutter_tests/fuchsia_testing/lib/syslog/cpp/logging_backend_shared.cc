// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "logging_backend_shared.h"

#include <inttypes.h>

#include <cstdio>

namespace syslog_backend {

void BeginRecordLegacy(LogBuffer* buffer, syslog::LogSeverity severity, const char* file,
                       unsigned int line, const char* msg, const char* condition) {
  auto header = MsgHeader::CreatePtr(buffer);
  header->buffer = buffer;
  header->Init(buffer, severity);
#ifndef __Fuchsia__
  auto severity_string = GetNameForLogSeverity(severity);
  header->WriteString(severity_string.data());
  header->WriteString(": ");
#endif
  header->WriteChar('[');
  header->WriteString(file);
  header->WriteChar('(');
  char a_buffer[128];
  snprintf(a_buffer, 128, "%i", line);
  header->WriteString(a_buffer);
  header->WriteString(")] ");
  if (condition) {
    header->WriteString("Check failed: ");
    header->WriteString(condition);
    header->WriteString(". ");
  }
  if (msg) {
    header->WriteString(msg);
    header->has_msg = true;
  }
}

// Common initialization for all KV pairs.
// Returns the header for writing the value.
MsgHeader* StartKv(LogBuffer* buffer, const char* key) {
  auto header = MsgHeader::CreatePtr(buffer);
  if (!header->first_kv || header->has_msg) {
    header->WriteChar(' ');
  }
  header->WriteString(key);
  header->WriteChar('=');
  header->first_kv = false;
  return header;
}

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, const char* value) {
  // "tag" has special meaning to our logging API
  if (strcmp("tag", key) == 0) {
    auto header = MsgHeader::CreatePtr(buffer);
    auto tag_size = strlen(value) + 1;
    header->user_tag = (reinterpret_cast<char*>(buffer->data) + sizeof(buffer->data)) - tag_size;
    memcpy(header->user_tag, value, tag_size);
    return;
  }
  auto header = StartKv(buffer, key);
  header->WriteChar('"');
  if (strchr(value, '"') != nullptr) {
    // Escape quotes in strings.
    size_t len = strlen(value);
    for (size_t i = 0; i < len; ++i) {
      char c = value[i];
      if (c == '"') {
        header->WriteChar('\\');
      }
      header->WriteChar(c);
    }
  } else {
    header->WriteString(value);
  }
  header->WriteChar('"');
}

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, int64_t value) {
  auto header = StartKv(buffer, key);
  char a_buffer[128];
  snprintf(a_buffer, 128, "%" PRId64, value);
  header->WriteString(a_buffer);
}

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, uint64_t value) {
  auto header = StartKv(buffer, key);
  char a_buffer[128];
  snprintf(a_buffer, 128, "%" PRIu64, value);
  header->WriteString(a_buffer);
}

void WriteKeyValueLegacy(LogBuffer* buffer, const char* key, double value) {
  auto header = StartKv(buffer, key);
  char a_buffer[128];
  snprintf(a_buffer, 128, "%f", value);
  header->WriteString(a_buffer);
}

void EndRecordLegacy(LogBuffer* buffer) {}

}  // namespace syslog_backend
