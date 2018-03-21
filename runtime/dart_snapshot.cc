// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_snapshot.h"

#include <sstream>

#include "flutter/fml/native_library.h"
#include "flutter/fml/paths.h"
#include "flutter/fml/trace_event.h"
#include "flutter/runtime/dart_snapshot_source.h"
#include "flutter/runtime/dart_vm.h"

namespace blink {

static const char* kVMDataSymbol = "kDartVmSnapshotData";
static const char* kVMInstructionsSymbol = "kDartVmSnapshotInstructions";
static const char* kIsolateDataSymbol = "kDartIsolateSnapshotData";
static const char* kIsolateInstructionsSymbol =
    "kDartIsolateSnapshotInstructions";

static std::unique_ptr<DartSnapshotSource> ResolveVMData(
    const Settings& settings) {
  if (settings.aot_snapshot_path.size() > 0) {
    auto path = fml::paths::JoinPaths(
        {settings.aot_snapshot_path, settings.aot_vm_snapshot_data_filename});
    if (auto source = DartSnapshotSource::CreateWithContentsOfFile(
            path.c_str(), false /* executable */)) {
      return source;
    }
  }

  auto loaded_process = fml::NativeLibrary::CreateForCurrentProcess();
  return DartSnapshotSource::CreateWithSymbolInLibrary(loaded_process,
                                                       kVMDataSymbol);
}

static std::unique_ptr<DartSnapshotSource> ResolveVMInstructions(
    const Settings& settings) {
  if (settings.aot_snapshot_path.size() > 0) {
    auto path = fml::paths::JoinPaths(
        {settings.aot_snapshot_path, settings.aot_vm_snapshot_instr_filename});
    if (auto source = DartSnapshotSource::CreateWithContentsOfFile(
            path.c_str(), true /* executable */)) {
      return source;
    }
  }

  if (settings.application_library_path.size() > 0) {
    auto library =
        fml::NativeLibrary::Create(settings.application_library_path.c_str());
    if (auto source = DartSnapshotSource::CreateWithSymbolInLibrary(
            library, kVMInstructionsSymbol)) {
      return source;
    }
  }

  auto loaded_process = fml::NativeLibrary::CreateForCurrentProcess();
  return DartSnapshotSource::CreateWithSymbolInLibrary(loaded_process,
                                                       kVMInstructionsSymbol);
}

static std::unique_ptr<DartSnapshotSource> ResolveIsolateData(
    const Settings& settings) {
  if (settings.aot_snapshot_path.size() > 0) {
    auto path =
        fml::paths::JoinPaths({settings.aot_snapshot_path,
                               settings.aot_isolate_snapshot_data_filename});
    if (auto source = DartSnapshotSource::CreateWithContentsOfFile(
            path.c_str(), false /* executable */)) {
      return source;
    }
  }

  auto loaded_process = fml::NativeLibrary::CreateForCurrentProcess();
  return DartSnapshotSource::CreateWithSymbolInLibrary(loaded_process,
                                                       kIsolateDataSymbol);
}

static std::unique_ptr<DartSnapshotSource> ResolveIsolateInstructions(
    const Settings& settings) {
  if (settings.aot_snapshot_path.size() > 0) {
    auto path =
        fml::paths::JoinPaths({settings.aot_snapshot_path,
                               settings.aot_isolate_snapshot_instr_filename});
    if (auto source = DartSnapshotSource::CreateWithContentsOfFile(
            path.c_str(), true /* executable */)) {
      return source;
    }
  }

  if (settings.application_library_path.size() > 0) {
    auto library =
        fml::NativeLibrary::Create(settings.application_library_path.c_str());
    if (auto source = DartSnapshotSource::CreateWithSymbolInLibrary(
            library, kIsolateInstructionsSymbol)) {
      return source;
    }
  }

  auto loaded_process = fml::NativeLibrary::CreateForCurrentProcess();
  return DartSnapshotSource::CreateWithSymbolInLibrary(
      loaded_process, kIsolateInstructionsSymbol);
}

std::unique_ptr<DartSnapshot> DartSnapshot::VMSnapshotFromSettings(
    const Settings& settings) {
  TRACE_EVENT0("flutter", "DartSnapshot::VMSnapshotFromSettings");
  auto snapshot =
      std::make_unique<DartSnapshot>(ResolveVMData(settings),         //
                                     ResolveVMInstructions(settings)  //
      );
  if (snapshot->IsValid()) {
    return snapshot;
  }
  return nullptr;
}

std::unique_ptr<DartSnapshot> DartSnapshot::IsolateSnapshotFromSettings(
    const Settings& settings) {
  TRACE_EVENT0("flutter", "DartSnapshot::IsolateSnapshotFromSettings");
  auto snapshot =
      std::make_unique<DartSnapshot>(ResolveIsolateData(settings),         //
                                     ResolveIsolateInstructions(settings)  //
      );
  if (snapshot->IsValid()) {
    return snapshot;
  }
  return nullptr;
}

DartSnapshot::DartSnapshot(std::unique_ptr<DartSnapshotSource> data,
                           std::unique_ptr<DartSnapshotSource> instructions)
    : data_(std::move(data)), instructions_(std::move(instructions)) {}

DartSnapshot::~DartSnapshot() = default;

bool DartSnapshot::IsValid() const {
  return static_cast<bool>(data_);
}

bool DartSnapshot::IsValidForAOT() const {
  return data_ && instructions_;
}

const DartSnapshotSource* DartSnapshot::GetData() const {
  return data_.get();
}

const DartSnapshotSource* DartSnapshot::GetInstructions() const {
  return instructions_.get();
}

const uint8_t* DartSnapshot::GetInstructionsIfPresent() const {
  return instructions_ ? instructions_->GetSnapshotPointer() : nullptr;
}

}  // namespace blink
