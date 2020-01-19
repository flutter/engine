// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_MAPPED_RESOURCE_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_MAPPED_RESOURCE_H_

#include <lib/fdio/namespace.h>

#include <string>

#include "third_party/dart/runtime/bin/elf_loader.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace fx {

class ElfSnapshot {
 public:
  ElfSnapshot() = default;
  ElfSnapshot(const ElfSnapshot&) = delete;
  ElfSnapshot(ElfSnapshot&& other) = default;
  ~ElfSnapshot();

  ElfSnapshot& operator=(const ElfSnapshot&) = delete;
  ElfSnapshot& operator=(ElfSnapshot&& other) = default;

  bool Load(fdio_ns_t* fdio_namespace, const std::string& path);
  bool Load(int dirfd, const std::string& path);

  const uint8_t* VmData() const { return vm_data_; }
  const uint8_t* VmInstrs() const { return vm_instrs_; }
  const uint8_t* IsolateData() const { return isolate_data_; }
  const uint8_t* IsolateInstrs() const { return isolate_instrs_; }

 private:
  bool Load(int fd);

  Dart_LoadedElf* handle_ = nullptr;

  const uint8_t* vm_data_ = nullptr;
  const uint8_t* vm_instrs_ = nullptr;
  const uint8_t* isolate_data_ = nullptr;
  const uint8_t* isolate_instrs_ = nullptr;
};

class MappedResource {
 public:
  static MappedResource MakeFileMapping(fdio_ns_t* namespc,
                                        const std::string& path,
                                        bool executable = false);
  static MappedResource MakeFileMapping(int dirfd,
                                        const std::string& path,
                                        bool executable = false);

  // Loads the content of a file from the given namespace and maps it into the
  // current process's address space. If namespace is null, the fdio "global"
  // namespace is used (in which case, ./pkg means the dart_runner's package).
  // The content is unmapped when the MappedResource goes out of scope. Returns
  // true on success.
  static bool LoadFromNamespace(fdio_ns_t* namespc,
                                const std::string& path,
                                MappedResource& resource,
                                bool executable = false);

  // Same as LoadFromNamespace, but takes a file descriptor to an opened
  // directory instead of a namespace.
  static bool LoadFromDir(int dirfd,
                          const std::string& path,
                          MappedResource& resource,
                          bool executable = false);

  MappedResource() {}
  MappedResource(const uint8_t* address, size_t size)
      : address_(address), size_(size) {}
  MappedResource(const MappedResource&) = delete;
  MappedResource(MappedResource&& other) = default;
  ~MappedResource();

  MappedResource& operator=(const MappedResource&) = delete;
  MappedResource& operator=(MappedResource&& other) = default;

  const uint8_t* address() const {
    return reinterpret_cast<const uint8_t*>(address_);
  }
  size_t size() const { return size_; }

 private:
  const void* address_ = nullptr;
  size_t size_ = 0;
};

}  // namespace fx

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_MAPPED_RESOURCE_H_
