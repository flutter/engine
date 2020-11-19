// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_SK_TRACE_MEMORY_DUMP_JSON_H_
#define FLUTTER_SHELL_COMMON_SK_TRACE_MEMORY_DUMP_JSON_H_

#define RAPIDJSON_HAS_STDSTRING 1

#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/fml/macros.h"
#include "rapidjson/document.h"
#include "third_party/skia/include/core/SkTraceMemoryDump.h"

namespace flutter {

/// Writes GrDirectContext::dumpMemoryStatistics data to a rapidjson::Document.
class SkTraceMemoryDumpJson : public SkTraceMemoryDump {
 public:
  SkTraceMemoryDumpJson();

  void dumpNumericValue(const char* dumpName,
                        const char* valueName,
                        const char* units,
                        uint64_t value) override;

  void dumpStringValue(const char* dumpName,
                       const char* valueName,
                       const char* value) override;

  void setMemoryBacking(const char* dumpName,
                        const char* backingType,
                        const char* backingObjectId) override;

  void setDiscardableMemoryBacking(
      const char* dumpName,
      const SkDiscardableMemory& discardableMemoryObject) override;

  LevelOfDetail getRequestedDetails() const override;

  bool shouldDumpWrappedObjects() const override;

  /// Call after passing this object to dumpMemoryStatistics to write the data
  /// to the specified document under the specified key_name.
  ///
  /// Calling this method will clear data recieved from dumpMemoryStatistics.
  void finish(std::string key_name, rapidjson::Document* response);

 private:
  std::unordered_map<std::string,
                     std::vector<std::pair<std::string, std::string>>>
      values_;

  FML_DISALLOW_COPY_AND_ASSIGN(SkTraceMemoryDumpJson);
};
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_SK_TRACE_MEMORY_DUMP_JSON_H_
