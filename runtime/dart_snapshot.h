// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_DART_SNAPSHOT_H_
#define FLUTTER_RUNTIME_DART_SNAPSHOT_H_

#include <memory>
#include <string>

#include "flutter/common/settings.h"
#include "flutter/runtime/dart_snapshot_source.h"
#include "lib/fxl/macros.h"

namespace blink {

class DartSnapshot {
 public:
  static std::unique_ptr<DartSnapshot> VMSnapshotFromSettings(
      const Settings& settings);

  static std::unique_ptr<DartSnapshot> IsolateSnapshotFromSettings(
      const Settings& settings);

  DartSnapshot(std::unique_ptr<DartSnapshotSource> data,
               std::unique_ptr<DartSnapshotSource> instructions);

  ~DartSnapshot();

  bool IsValid() const;

  bool IsValidForAOT() const;

  const DartSnapshotSource* GetData() const;

  const DartSnapshotSource* GetInstructions() const;

  const uint8_t* GetInstructionsIfPresent() const;

 private:
  std::unique_ptr<DartSnapshotSource> data_;
  std::unique_ptr<DartSnapshotSource> instructions_;

  FXL_DISALLOW_COPY_AND_ASSIGN(DartSnapshot);
};

}  // namespace blink

#endif  // FLUTTER_RUNTIME_DART_SNAPSHOT_H_
