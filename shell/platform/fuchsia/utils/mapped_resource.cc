// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/utils/mapped_resource.h"

#include <fcntl.h>
#include <fuchsia/io/cpp/fidl.h>
#include <fuchsia/mem/cpp/fidl.h>
#include <lib/fdio/directory.h>
#include <lib/trace/event.h>
#include <zircon/status.h>

#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/shell/platform/fuchsia/utils/vmo.h"

namespace fx {
namespace {

bool OpenVmoCommon(fuchsia::mem::Buffer* resource_vmo,
                   int dirfd,
                   const std::string& path,
                   bool executable) {
  bool result;
  if (dirfd == -1) {
    // Opening a file in the root namespace expects an absolute path.
    FX_CHECK(path[0] == '/');
    result = fx::VmoFromFilename(path, executable, resource_vmo);
  } else {
    // openat of a path with a leading '/' ignores the namespace fd.
    // require a relative path.
    FX_CHECK(path[0] != '/');
    result = fx::VmoFromFilenameAt(dirfd, path, executable, resource_vmo);
  }

  if (dirfd != -1) {
    close(dirfd);
  }

  return result;
}

bool OpenVmo(fuchsia::mem::Buffer* resource_vmo,
             int dirfd,
             const std::string& path,
             bool executable) {
  TRACE_DURATION("dart", "OpenVmo", "path", path);
  FX_DCHECK(dirfd >= 0);

  return OpenVmoCommon(resource_vmo, dirfd, path, executable);
}

bool OpenVmo(fuchsia::mem::Buffer* resource_vmo,
             fdio_ns_t* fdio_namespace,
             const std::string& path,
             bool executable) {
  TRACE_DURATION("dart", "OpenVmo", "path", path);

  int root_dir = -1;
  if (fdio_namespace != nullptr) {
    auto root_dir = fdio_ns_opendir(fdio_namespace);
    if (root_dir < 0) {
      FX_LOG(ERROR) << "Failed to open namespace directory";
      return false;
    }
  }

  return OpenVmoCommon(resource_vmo, root_dir, path, executable);
}

bool MapVmo(const std::string& path,
            fuchsia::mem::Buffer resource_vmo,
            MappedResource& resource,
            bool executable) {
  TRACE_DURATION("dart", "MapVmo", "path", path);

  if (resource_vmo.size == 0) {
    resource = MappedResource();

    return true;
  }

  uint32_t flags = ZX_VM_PERM_READ;
  if (executable) {
    flags |= ZX_VM_PERM_EXECUTE;
  }

  uintptr_t addr;
  zx_status_t status = zx::vmar::root_self()->map(
      0, resource_vmo.vmo, 0, resource_vmo.size, flags, &addr);
  if (status != ZX_OK) {
    FX_LOG(ERROR) << "Failed to map: " << zx_status_get_string(status);
    resource = MappedResource();

    return false;
  }

  resource =
      MappedResource(reinterpret_cast<uint8_t*>(addr), resource_vmo.size);

  return true;
}

bool UnmapVmo(const void* address, size_t size) {
  zx_status_t status = zx::vmar::root_self()->unmap(
      reinterpret_cast<const uintptr_t>(address), size);
  if (status != ZX_OK) {
    FX_LOG(ERROR) << "Failed to unmap: " << zx_status_get_string(status);

    return false;
  }

  return true;
}

int OpenFdExec(const std::string& path, int dirfd) {
  int fd = -1;
  zx_status_t result;
  if (dirfd == AT_FDCWD) {
    // fdio_open_fd_at does not support AT_FDCWD, by design.  Use fdio_open_fd
    // and expect an absolute path for that usage pattern.
    FX_CHECK(path[0] == '/');
    result = fdio_open_fd(
        path.c_str(),
        fuchsia::io::OPEN_RIGHT_READABLE | fuchsia::io::OPEN_RIGHT_EXECUTABLE,
        &fd);
  } else {
    FX_CHECK(path[0] != '/');
    result = fdio_open_fd_at(
        dirfd, path.c_str(),
        fuchsia::io::OPEN_RIGHT_READABLE | fuchsia::io::OPEN_RIGHT_EXECUTABLE,
        &fd);
  }
  if (result != ZX_OK) {
    FX_LOG(ERROR) << "fdio_open_fd_at(" << path.c_str()
                  << ") failed: " << zx_status_get_string(result);
    return -1;
  }
  return fd;
}

}  // namespace

ElfSnapshot::~ElfSnapshot() {
  Dart_UnloadELF(handle_);
}

bool ElfSnapshot::Load(fdio_ns_t* fdio_namespace, const std::string& path) {
  int root_dir = -1;
  if (fdio_namespace == nullptr) {
    root_dir = AT_FDCWD;
  } else {
    root_dir = fdio_ns_opendir(fdio_namespace);
    if (root_dir < 0) {
      FX_LOG(ERROR) << "Failed to open namespace directory";
      return false;
    }
  }
  return Load(root_dir, path);
}

bool ElfSnapshot::Load(int dirfd, const std::string& path) {
  const int fd = OpenFdExec(path, dirfd);
  if (fd < 0) {
    FX_LOG(ERROR) << "Failed to open VMO for %s from dir." << path.c_str();
    return false;
  }
  return Load(fd);
}

bool ElfSnapshot::Load(int fd) {
  const char* error;
  handle_ = Dart_LoadELF_Fd(fd, 0, &error, &vm_data_, &vm_instrs_,
                            &isolate_data_, &isolate_instrs_);
  close(fd);

  if (handle_ == nullptr) {
    FX_LOG(ERROR) << "Failed load ELF: " << error;
  }
  return handle_ != nullptr;
}

MappedResource MappedResource::MakeFileMapping(fdio_ns_t* namespc,
                                               const std::string& path,
                                               bool executable) {
  fx::MappedResource mapping;
  bool success =
      fx::MappedResource::LoadFromNamespace(namespc, path, mapping, executable);
  if (!success) {
    FX_LOG(ERROR) << "Failed creating MappedResource for " << path.c_str();
  }

  return mapping;
}

MappedResource MappedResource::MakeFileMapping(int dirfd,
                                               const std::string& path,
                                               bool executable) {
  fx::MappedResource mapping;
  bool success =
      fx::MappedResource::LoadFromDir(dirfd, path, mapping, executable);
  if (!success) {
    FX_LOG(ERROR) << "Failed creating MappedResource for " << path.c_str();
  }

  return mapping;
}

MappedResource::~MappedResource() {
  if (address_ != nullptr && size_ != 0) {
    UnmapVmo(address_, size_);
  }
}

bool MappedResource::LoadFromNamespace(fdio_ns_t* fdio_namespace,
                                       const std::string& path,
                                       MappedResource& resource,
                                       bool executable) {
  TRACE_DURATION("dart", "LoadFromNamespace", "path", path);

  fuchsia::mem::Buffer resource_vmo;
  return OpenVmo(&resource_vmo, fdio_namespace, path, executable) &&
         MapVmo(path, std::move(resource_vmo), resource, executable);
}

bool MappedResource::LoadFromDir(int dirfd,
                                 const std::string& path,
                                 MappedResource& resource,
                                 bool executable) {
  TRACE_DURATION("dart", "LoadFromDir", "path", path);

  fuchsia::mem::Buffer resource_vmo;
  return OpenVmo(&resource_vmo, dirfd, path, executable) &&
         MapVmo(path, std::move(resource_vmo), resource, executable);
}

}  // namespace fx
