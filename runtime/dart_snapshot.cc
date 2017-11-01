// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/runtime/dart_snapshot.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "lib/fxl/build_config.h"
#include "lib/fxl/files/eintr_wrapper.h"
#include "lib/fxl/files/unique_fd.h"
#include "lib/fxl/logging.h"

extern "C" {
extern const uint8_t kDartVmSnapshotData[];
extern const uint8_t kDartVmSnapshotInstructions[];
extern const uint8_t kDartIsolateCoreSnapshotData[];
extern const uint8_t kDartIsolateCoreSnapshotInstructions[];
}  // extern "C"

namespace blink {

std::unique_ptr<DartSnapshot> DartSnapshot::FromSettings(
    const Settings& settings) {
#if !FLUTTER_AOT
  return FromSymbolsInCurrentExecutable();
#elif OS_IOS
  return FromSymbolsInDynamicLibrary(
      settings.application_library_path.empty()
          ? "App.framework/App"
          : settings.application_library_path.c_str());
#elif OS_ANDROID
  return FromSymbolsInAOTSnapshotAtPath(
      settings.aot_snapshot_path,                   //
      settings.aot_vm_snapshot_data_filename,       //
      settings.aot_vm_snapshot_instr_filename,      //
      settings.aot_isolate_snapshot_data_filename,  //
      settings.aot_isolate_snapshot_instr_filename  //
  );
#else
  FXL_DLOG(INFO) << "Cannot infer snapshot location for this platform";
  return nullptr;
#endif
}

std::unique_ptr<DartSnapshot> DartSnapshot::FromSymbolsInCurrentExecutable() {
  return FromSnapshotPointers(::kDartVmSnapshotData,                  //
                              ::kDartVmSnapshotInstructions,          //
                              ::kDartIsolateCoreSnapshotData,         //
                              ::kDartIsolateCoreSnapshotInstructions  //
  );
}

std::unique_ptr<DartSnapshot> DartSnapshot::FromSymbolsInDynamicLibrary(
    const char* library_path) {
  if (library_path == nullptr) {
    return nullptr;
  }

  ::dlerror();  // clear previous errors on thread

  // Attempt to open the library and grab pointers at known locations it.
  void* library_handle = ::dlopen(library_path, RTLD_NOW);

  const char* err = dlerror();

  if (err != nullptr) {
    FXL_LOG(ERROR) << "dlopen failed: " << err;
    return nullptr;
  }

  return FromSnapshotPointers(
      reinterpret_cast<const uint8_t*>(
          ::dlsym(library_handle, "kDartVmSnapshotData")),
      reinterpret_cast<const uint8_t*>(
          ::dlsym(library_handle, "kDartVmSnapshotInstructions")),
      reinterpret_cast<const uint8_t*>(
          ::dlsym(library_handle, "kDartIsolateSnapshotData")),
      reinterpret_cast<const uint8_t*>(
          ::dlsym(library_handle, "kDartIsolateSnapshotInstructions")));
}

static const uint8_t* GetMemoryMappedSnapshot(
    const std::string& snapshot_path,
    const std::string& default_file_name,
    const std::string& settings_file_name,
    bool executable) {
  std::string asset_path;
  if (settings_file_name.empty()) {
    asset_path = snapshot_path + "/" + default_file_name;
  } else {
    asset_path = snapshot_path + "/" + settings_file_name;
  }

  struct stat info = {};
  if (stat(asset_path.c_str(), &info) < 0) {
    return nullptr;
  }
  int64_t asset_size = info.st_size;

  fxl::UniqueFD fd(HANDLE_EINTR(::open(asset_path.c_str(), O_RDONLY)));
  if (fd.get() == -1) {
    return nullptr;
  }

  int mmap_flags = PROT_READ;
  if (executable) {
    mmap_flags |= PROT_EXEC;
  }
  void* symbol = ::mmap(NULL, asset_size, mmap_flags, MAP_PRIVATE, fd.get(), 0);
  if (symbol == MAP_FAILED) {
    return nullptr;
  }

  return reinterpret_cast<const uint8_t*>(symbol);
}

std::unique_ptr<DartSnapshot> DartSnapshot::FromSymbolsInAOTSnapshotAtPath(
    const std::string& snapshot_path,
    const std::string& vm_snapshot_data_filename,
    const std::string& vm_snapshot_instructions_filename,
    const std::string& vm_isolate_snapshot_data_filename,
    const std::string& vm_isolate_snapshot_instructions_filename) {
  if (snapshot_path.empty()) {
    return nullptr;
  }

  return FromSnapshotPointers(
      GetMemoryMappedSnapshot(snapshot_path, "vm_snapshot_data",
                              vm_snapshot_data_filename, false),
      GetMemoryMappedSnapshot(snapshot_path, "vm_snapshot_instr",
                              vm_snapshot_instructions_filename, true),
      GetMemoryMappedSnapshot(snapshot_path, "isolate_snapshot_data",
                              vm_isolate_snapshot_data_filename, false),
      GetMemoryMappedSnapshot(snapshot_path, "isolate_snapshot_instr",
                              vm_isolate_snapshot_instructions_filename, true));
}

std::unique_ptr<DartSnapshot> DartSnapshot::FromSnapshotPointers(
    const uint8_t* vm_snapshot_data,
    const uint8_t* vm_snapshot_instructions,
    const uint8_t* default_isolate_snapshot_data,
    const uint8_t* default_isolate_snapshot_instructions) {
  std::unique_ptr<DartSnapshot> snapshot(new DartSnapshot(
      vm_snapshot_data,                      //
      vm_snapshot_instructions,              //
      default_isolate_snapshot_data,         //
      default_isolate_snapshot_instructions  //
      ));
  if (!snapshot->IsValid()) {
    return nullptr;
  }
  return snapshot;
}

DartSnapshot::DartSnapshot(const uint8_t* vm_snapshot_data,
                           const uint8_t* vm_snapshot_instructions,
                           const uint8_t* default_isolate_snapshot_data,
                           const uint8_t* default_isolate_snapshot_instructions)
    : vm_snapshot_data_(vm_snapshot_data),
      vm_snapshot_instructions_(vm_snapshot_instructions),
      default_isolate_snapshot_data_(default_isolate_snapshot_data),
      default_isolate_snapshot_instructions_(
          default_isolate_snapshot_instructions) {}

DartSnapshot::~DartSnapshot() = default;

bool DartSnapshot::IsValid() const {
  return vm_snapshot_data_ != nullptr &&               //
         vm_snapshot_instructions_ != nullptr &&       //
         default_isolate_snapshot_data_ != nullptr &&  //
         default_isolate_snapshot_instructions_ != nullptr;
}

const uint8_t* DartSnapshot::GetVMSnapshotData() const {
  return vm_snapshot_data_;
}

const uint8_t* DartSnapshot::GetVMSnapshotInstructions() const {
  return vm_snapshot_instructions_;
}

const uint8_t* DartSnapshot::GetDefaultIsolateSnapshotData() const {
  return default_isolate_snapshot_data_;
}

const uint8_t* DartSnapshot::GetDefaultIsolateSnapshotInstructions() const {
  return default_isolate_snapshot_instructions_;
}

}  // namespace blink
