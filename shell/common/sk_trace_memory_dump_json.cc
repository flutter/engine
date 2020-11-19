// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/sk_trace_memory_dump_json.h"

namespace flutter {

SkTraceMemoryDumpJson::SkTraceMemoryDumpJson() {}

void SkTraceMemoryDumpJson::dumpNumericValue(const char* dumpName,
                                             const char* valueName,
                                             const char* units,
                                             uint64_t value) {
  values_[std::string(dumpName)].push_back(
      std::make_pair(std::string(valueName), std::to_string(value)));
  values_[std::string(dumpName)].push_back(
      std::make_pair("units", std::string(units)));
}

void SkTraceMemoryDumpJson::dumpStringValue(const char* dumpName,
                                            const char* valueName,
                                            const char* value) {
  values_[std::string(dumpName)].push_back(
      std::make_pair(std::string(valueName), std::string(value)));
}

void SkTraceMemoryDumpJson::setMemoryBacking(const char* dumpName,
                                             const char* backingType,
                                             const char* backingObjectId) {}

void SkTraceMemoryDumpJson::setDiscardableMemoryBacking(
    const char* dumpName,
    const SkDiscardableMemory& discardableMemoryObject) {}

SkTraceMemoryDump::LevelOfDetail SkTraceMemoryDumpJson::getRequestedDetails()
    const {
  return SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail;
}

bool SkTraceMemoryDumpJson::shouldDumpWrappedObjects() const {
  return true;
}

void SkTraceMemoryDumpJson::finish(std::string key_name,
                                   rapidjson::Document* response) {
  auto& allocator = response->GetAllocator();
  rapidjson::Value object(rapidjson::kObjectType);
  for (const auto& [key, value] : values_) {
    rapidjson::Value json_value(rapidjson::kObjectType);
    for (const auto& pair : value) {
      json_value.AddMember(rapidjson::Value(pair.first, allocator).Move(),
                           rapidjson::Value(pair.second, allocator).Move(),
                           allocator);
    }
    object.AddMember(rapidjson::Value(key, allocator).Move(), json_value,
                     allocator);
  }
  values_.clear();
  response->AddMember(rapidjson::Value(key_name, allocator).Move(), object,
                      allocator);
}

}  // namespace flutter
