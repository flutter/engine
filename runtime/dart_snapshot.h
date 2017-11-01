// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_DART_SNAPSHOT_H_
#define FLUTTER_RUNTIME_DART_SNAPSHOT_H_

#include <memory>
#include <string>

#include "flutter/common/settings.h"
#include "lib/fxl/macros.h"

namespace blink {

class DartSnapshot {
 public:
  static std::unique_ptr<DartSnapshot> FromSettings(const Settings& settings);

  static std::unique_ptr<DartSnapshot> FromSymbolsInCurrentExecutable();

  static std::unique_ptr<DartSnapshot> FromSymbolsInDynamicLibrary(
      const char* dynamic_library_path);

  static std::unique_ptr<DartSnapshot> FromSymbolsInAOTSnapshotAtPath(
      const std::string& snapshot_path,
      const std::string& vm_snapshot_data_filename,
      const std::string& vm_snapshot_instructions_filename,
      const std::string& vm_isolate_snapshot_data_filename,
      const std::string& vm_isolate_snapshot_instructions_filename);

  static std::unique_ptr<DartSnapshot> FromSnapshotPointers(
      const uint8_t* vm_snapshot_data,
      const uint8_t* vm_snapshot_instructions,
      const uint8_t* default_isolate_snapshot_data,
      const uint8_t* default_isolate_snapshot_instructions);

  ~DartSnapshot();

  bool IsValid() const;

  const uint8_t* GetVMSnapshotData() const;

  const uint8_t* GetVMSnapshotInstructions() const;

  const uint8_t* GetDefaultIsolateSnapshotData() const;

  const uint8_t* GetDefaultIsolateSnapshotInstructions() const;

 private:
  const uint8_t* vm_snapshot_data_;
  const uint8_t* vm_snapshot_instructions_;
  const uint8_t* default_isolate_snapshot_data_;
  const uint8_t* default_isolate_snapshot_instructions_;

  DartSnapshot(const uint8_t* vm_snapshot_data,
               const uint8_t* vm_snapshot_instructions,
               const uint8_t* default_isolate_snapshot_data,
               const uint8_t* default_isolate_snapshot_instructions);

  FXL_DISALLOW_COPY_AND_ASSIGN(DartSnapshot);
};

}  // namespace blink

#endif  // FLUTTER_RUNTIME_DART_SNAPSHOT_H_
