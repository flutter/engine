// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_vm_data.h"

namespace blink {

std::shared_ptr<const DartVMData> DartVMData::Create(
    Settings settings,
    fml::RefPtr<DartSnapshot> vm_snapshot,
    fml::RefPtr<DartSnapshot> isolate_snapshot,
    fml::RefPtr<DartSnapshot> shared_snapshot) {
  if (!vm_snapshot || !vm_snapshot->IsValid()) {
    // Caller did not provide a valid VM snapshot. Attempt to infer one
    // from the settings.
    vm_snapshot = DartSnapshot::VMSnapshotFromSettings(settings);
    if (!vm_snapshot) {
      FML_LOG(ERROR)
          << "VM snapshot invalid and could not be inferred from settings.";
      return {};
    }
  }

  if (!isolate_snapshot || !isolate_snapshot->IsValid()) {
    // Caller did not provide a valid isolate snapshot. Attempt to infer one
    // from the settings.
    isolate_snapshot = DartSnapshot::IsolateSnapshotFromSettings(settings);
    if (!isolate_snapshot) {
      FML_LOG(ERROR) << "Isolate snapshot invalid and could not be inferred "
                        "from settings.";
      return {};
    }
  }

  if (!shared_snapshot || !shared_snapshot->IsValid()) {
    shared_snapshot = DartSnapshot::Empty();
    if (!shared_snapshot) {
      FML_LOG(ERROR) << "Shared snapshot invalid.";
      return {};
    }
  }

  return std::shared_ptr<const DartVMData>(new DartVMData(
      std::move(settings),          //
      std::move(vm_snapshot),       //
      std::move(isolate_snapshot),  //
      std::move(shared_snapshot)    //
      ));
}

static std::unique_ptr<fml::Thread> CreateServiceProtocolThread(
    const Settings& settings) {
#if (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_RELEASE) || \
    (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DYNAMIC_RELEASE)
  return nullptr;
#else
  if (!settings.enable_observatory) {
    return nullptr;
  }
  return std::make_unique<fml::Thread>("io.flutter.service");
#endif
}

DartVMData::DartVMData(Settings settings,
                       fml::RefPtr<const DartSnapshot> vm_snapshot,
                       fml::RefPtr<const DartSnapshot> isolate_snapshot,
                       fml::RefPtr<const DartSnapshot> shared_snapshot)
    : settings_(settings),
      vm_snapshot_(vm_snapshot),
      isolate_snapshot_(isolate_snapshot),
      shared_snapshot_(shared_snapshot),
      service_protocol_thread_(CreateServiceProtocolThread(settings_)) {}

DartVMData::~DartVMData() = default;

const Settings& DartVMData::GetSettings() const {
  return settings_;
}

const DartSnapshot& DartVMData::GetVMSnapshot() const {
  return *vm_snapshot_;
}

fml::RefPtr<const DartSnapshot> DartVMData::GetIsolateSnapshot() const {
  return isolate_snapshot_;
}

fml::RefPtr<const DartSnapshot> DartVMData::GetSharedSnapshot() const {
  return shared_snapshot_;
}

fml::RefPtr<fml::TaskRunner> DartVMData::GetServiceTaskRunner() const {
  if (!service_protocol_thread_) {
    return nullptr;
  }
  return service_protocol_thread_->GetTaskRunner();
}

}  // namespace blink
