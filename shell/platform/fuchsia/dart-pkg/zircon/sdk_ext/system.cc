// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/system.h"

#include <fcntl.h>  // For open, openat, close
#include <lib/fdio/fd.h>
#include <lib/fdio/io.h>
#include <lib/fdio/namespace.h>
#include <lib/zx/channel.h>
#include <lib/zx/clock.h>
#include <lib/zx/eventpair.h>
#include <lib/zx/handle.h>
#include <lib/zx/socket.h>
#include <lib/zx/time.h>
#include <lib/zx/vmar.h>
#include <lib/zx/vmo.h>
#include <sys/stat.h>  // For fstat
#include <zircon/types.h>

#include <array>

#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/dart_binding_macros.h"
#include "flutter/third_party/tonic/dart_class_library.h"
#include "flutter/third_party/tonic/dart_list.h"

namespace zircon::dart {
namespace {

constexpr char kGetSizeResult[] = "GetSizeResult";
constexpr char kHandlePairResult[] = "HandlePairResult";
constexpr char kHandleResult[] = "HandleResult";
constexpr char kReadResult[] = "ReadResult";
constexpr char kWriteResult[] = "WriteResult";
constexpr char kFromFileResult[] = "FromFileResult";
constexpr char kMapResult[] = "MapResult";

struct SizedRegion {
  SizedRegion(void* r, size_t s) : region(r), size(s) {}

  void* region;
  size_t size;
};

class ByteDataScope {
 public:
  explicit ByteDataScope(Dart_Handle dart_handle) : dart_handle_(dart_handle) {
    Acquire();
  }

  explicit ByteDataScope(size_t size) {
    dart_handle_ = Dart_NewTypedData(Dart_TypedData_kByteData, size);
    FX_DCHECK(!tonic::LogIfError(dart_handle_));
    Acquire();
    FX_DCHECK(size == size_);
  }

  ~ByteDataScope() {
    if (is_valid_) {
      Release();
    }
  }

  void* data() const { return data_; }
  size_t size() const { return size_; }
  Dart_Handle dart_handle() const { return dart_handle_; }
  bool isValid() const { return is_valid_; }

  void Release() {
    FX_DCHECK(is_valid_);
    Dart_Handle result = Dart_TypedDataReleaseData(dart_handle_);
    tonic::LogIfError(result);
    is_valid_ = false;
    data_ = nullptr;
    size_ = 0;
  }

 private:
  void Acquire() {
    FX_DCHECK(size_ == 0);
    FX_DCHECK(data_ == nullptr);
    FX_DCHECK(!is_valid_);

    Dart_TypedData_Type type;
    intptr_t size;
    Dart_Handle result =
        Dart_TypedDataAcquireData(dart_handle_, &type, &data_, &size);
    is_valid_ =
        !tonic::LogIfError(result) && type == Dart_TypedData_kByteData && data_;
    if (is_valid_) {
      size_ = size;
    } else {
      size_ = 0;
    }
  }

  Dart_Handle dart_handle_;
  bool is_valid_ = false;
  size_t size_ = 0;
  void* data_ = nullptr;
};

template <class... Args>
Dart_Handle ConstructDartObject(const char* class_name, Args&&... args) {
  tonic::DartClassLibrary& class_library =
      tonic::DartState::Current()->class_library();
  Dart_Handle type =
      Dart_HandleFromPersistent(class_library.GetClass("zircon", class_name));
  FX_DCHECK(!tonic::LogIfError(type));

  const char* cstr;
  Dart_StringToCString(Dart_ToString(type), &cstr);

  std::array<Dart_Handle, sizeof...(Args)> args_array{
      {std::forward<Args>(args)...}};
  Dart_Handle object =
      Dart_New(type, Dart_EmptyString(), sizeof...(Args), args_array.data());
  FX_DCHECK(!tonic::LogIfError(object));
  return object;
}

fdio_ns_t* GetNamespace() {
  // Grab the fdio_ns_t* out of the isolate.
  Dart_Handle zircon_lib = Dart_LookupLibrary(tonic::ToDart("dart:zircon"));
  FX_DCHECK(!tonic::LogIfError(zircon_lib));
  Dart_Handle namespace_type =
      Dart_GetType(zircon_lib, tonic::ToDart("_Namespace"), 0, nullptr);
  FX_DCHECK(!tonic::LogIfError(namespace_type));
  Dart_Handle namespace_field =
      Dart_GetField(namespace_type, tonic::ToDart("_namespace"));
  FX_DCHECK(!tonic::LogIfError(namespace_field));

  return reinterpret_cast<fdio_ns_t*>(
      tonic::DartConverter<intptr_t>::FromDart(namespace_field));
}

int FdFromPath(std::string path) {
  // Get a VMO for the file.
  fdio_ns_t* ns = reinterpret_cast<fdio_ns_t*>(GetNamespace());
  int dirfd = fdio_ns_opendir(ns);
  int out_fd = -1;
  if (dirfd != -1) {
    const char* c_path = path.c_str();
    if (path.length() > 0 && c_path[0] == '/') {
      c_path = &c_path[1];
    }
    out_fd = openat(dirfd, c_path, O_RDONLY);

    close(dirfd);
  }

  return out_fd;
}

zx::channel CloneChannelFromFileDescriptor(int fd) {
  zx::channel channel;
  if (fdio_fd_clone(fd, channel.reset_and_get_address()) != ZX_OK) {
    return zx::channel();
  }

  zx_info_handle_basic_t info = {};
  if (channel.get_info(ZX_INFO_HANDLE_BASIC, &info, sizeof(info), nullptr,
                       nullptr) != ZX_OK ||
      info.type != ZX_OBJ_TYPE_CHANNEL) {
    return zx::channel();
  }

  return channel;
}

void VmoMapFinalizer(void* isolate_callback_data,
                     Dart_WeakPersistentHandle handle,
                     void* peer) {
  SizedRegion* r = reinterpret_cast<SizedRegion*>(peer);
  zx::vmar::root_self()->unmap(reinterpret_cast<uintptr_t>(r->region), r->size);

  delete r;
}

}  // namespace

IMPLEMENT_WRAPPERTYPEINFO(zircon, System);

#define FOR_EACH_STATIC_BINDING(V) \
  V(System, channelCreate)         \
  V(System, channelFromFile)       \
  V(System, channelWrite)          \
  V(System, channelQueryAndRead)   \
  V(System, connectToService)      \
  V(System, clockGet)              \
  V(System, eventpairCreate)       \
  V(System, socketCreate)          \
  V(System, socketRead)            \
  V(System, socketWrite)           \
  V(System, vmoCreate)             \
  V(System, vmoFromFile)           \
  V(System, vmoGetSize)            \
  V(System, vmoSetSize)            \
  V(System, vmoMap)                \
  V(System, vmoRead)               \
  V(System, vmoWrite)

FOR_EACH_STATIC_BINDING(DART_NATIVE_CALLBACK_STATIC)

void System::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({FOR_EACH_STATIC_BINDING(DART_REGISTER_NATIVE_STATIC)});
}

Dart_Handle System::channelCreate(uint32_t options) {
  FX_DCHECK(options >= 0);

  zx::channel out0, out1;
  zx_status_t status = zx::channel::create(options, &out0, &out1);
  if (status != ZX_OK) {
    return ConstructDartObject(kHandlePairResult, tonic::ToDart(status));
  } else {
    return ConstructDartObject(kHandlePairResult, tonic::ToDart(status),
                               tonic::ToDart(Handle::Create(std::move(out0))),
                               tonic::ToDart(Handle::Create(std::move(out1))));
  }
}

Dart_Handle System::channelFromFile(std::string path) {
  int fd = FdFromPath(path);
  if (fd == -1) {
    return ConstructDartObject(kHandleResult, tonic::ToDart(ZX_ERR_IO));
  }

  // Get channel from fd.
  zx::channel channel = CloneChannelFromFileDescriptor(fd);
  if (!channel) {
    return ConstructDartObject(kHandleResult, tonic::ToDart(ZX_ERR_IO));
  }

  return ConstructDartObject(kHandleResult, tonic::ToDart(ZX_OK),
                             tonic::ToDart(Handle::Create(std::move(channel))));
}

zx_status_t System::channelWrite(std::shared_ptr<Handle> channel,
                                 const tonic::DartByteData& data,
                                 std::vector<std::shared_ptr<Handle>> handles) {
  if (!channel || !channel->isValid()) {
    data.Release();
    return ZX_ERR_BAD_HANDLE;
  }

  std::vector<zx_handle_t> zx_handles;
  for (auto& handle : handles) {
    zx_handles.emplace_back(handle->ReleaseHandle().release());
    handle.reset();
  }

  zx::unowned<zx::channel> zx_channel(channel->handle());
  zx_status_t status = zx_channel->write(0, data.data(), data.length_in_bytes(),
                                         zx_handles.data(), zx_handles.size());

  data.Release();
  return status;
}

Dart_Handle System::channelQueryAndRead(std::shared_ptr<Handle> channel) {
  if (!channel || !channel->isValid()) {
    return ConstructDartObject(kReadResult, tonic::ToDart(ZX_ERR_BAD_HANDLE));
  }

  // Query the size of the next message.
  uint32_t actual_bytes = 0;
  uint32_t actual_handles = 0;
  zx::unowned<zx::channel> zx_channel(channel->handle());
  zx_status_t write_status = zx_channel->read(0, nullptr, nullptr, 0, 0,
                                              &actual_bytes, &actual_handles);
  if (write_status != ZX_ERR_BUFFER_TOO_SMALL) {
    // An empty message or an error.
    return ConstructDartObject(kReadResult, tonic::ToDart(write_status));
  }

  // Allocate space for the bytes and handles.
  ByteDataScope bytes(actual_bytes);
  FX_DCHECK(bytes.isValid());
  std::vector<zx_handle_t> handles(actual_handles);

  // Make the call to actually get the message.
  zx_status_t read_status =
      zx_channel->read(0, bytes.data(), handles.data(), bytes.size(),
                       handles.size(), &actual_bytes, &actual_handles);
  FX_DCHECK(read_status != ZX_OK || bytes.size() == actual_bytes);

  Dart_Handle result;
  if (read_status == ZX_OK) {
    FX_DCHECK(handles.size() == actual_handles);

    // return a ReadResult object.
    std::vector<std::shared_ptr<Handle>> handles_as_dart_obj(actual_handles);
    for (auto handle : handles) {
      handles_as_dart_obj.emplace_back(Handle::Create(handle));
    }
    result =
        ConstructDartObject(kReadResult, tonic::ToDart(read_status),
                            bytes.dart_handle(), tonic::ToDart(actual_bytes),
                            tonic::ToDart(std::move(handles_as_dart_obj)));
  } else {
    result = ConstructDartObject(kReadResult, tonic::ToDart(read_status));
  }
  bytes.Release();

  return result;
}

zx_status_t System::connectToService(std::string path,
                                     std::shared_ptr<Handle> channel) {
  FX_DCHECK(channel.get());
  FX_DCHECK(channel->isValid());

  // TODO (kaushikiska): Once fuchsia adds fs to their sdk,
  // use the rights macros from "fs/vfs.h"
  constexpr int32_t ZX_FS_RIGHT_READABLE = 0x1;  // The file may be read.
  constexpr int32_t ZX_FS_RIGHT_WRITABLE = 0x2;  // The file may be written.
  return fdio_ns_connect(GetNamespace(), path.c_str(),
                         ZX_FS_RIGHT_READABLE | ZX_FS_RIGHT_WRITABLE,
                         channel->ReleaseHandle().get());
}

zx_time_t System::clockGet(zx_clock_t clock_id) {
  FX_DCHECK(clock_id == ZX_CLOCK_MONOTONIC || clock_id == ZX_CLOCK_UTC ||
            clock_id == ZX_CLOCK_THREAD);

  zx_status_t status = ZX_OK;
  zx_time_t time = ZX_TIME_INFINITE;
  switch (clock_id) {
    case ZX_CLOCK_MONOTONIC: {
      zx::time time_mono;
      status = zx::clock::get(&time_mono);
      time = time_mono.get();
    }
    case ZX_CLOCK_UTC: {
      zx::time_utc time_utc;
      status = zx::clock::get(&time_utc);
      time = time_utc.get();
    }
    case ZX_CLOCK_THREAD: {
      zx::time_thread time_thread;
      status = zx::clock::get(&time_thread);
      time = time_thread.get();
    }
  }

  return time;
}

Dart_Handle System::eventpairCreate(uint32_t options) {
  zx::eventpair out0, out1;
  zx_status_t status = zx::eventpair::create(0, &out0, &out1);
  if (status != ZX_OK) {
    return ConstructDartObject(kHandlePairResult, tonic::ToDart(status));
  } else {
    return ConstructDartObject(kHandlePairResult, tonic::ToDart(status),
                               tonic::ToDart(Handle::Create(std::move(out0))),
                               tonic::ToDart(Handle::Create(std::move(out1))));
  }
}

Dart_Handle System::socketCreate(uint32_t options) {
  zx::socket out0, out1;
  zx_status_t status = zx::socket::create(options, &out0, &out1);
  if (status != ZX_OK) {
    return ConstructDartObject(kHandlePairResult, tonic::ToDart(status));
  } else {
    return ConstructDartObject(kHandlePairResult, tonic::ToDart(status),
                               tonic::ToDart(Handle::Create(std::move(out0))),
                               tonic::ToDart(Handle::Create(std::move(out1))));
  }
}

Dart_Handle System::socketRead(std::shared_ptr<Handle> socket, size_t size) {
  if (!socket || !socket->isValid()) {
    return ConstructDartObject(kReadResult, tonic::ToDart(ZX_ERR_BAD_HANDLE));
  }

  ByteDataScope bytes(size);
  size_t actual_bytes;
  zx::unowned<zx::socket> zx_socket(socket->handle());
  zx_status_t read_status =
      zx_socket->read(0, bytes.data(), size, &actual_bytes);

  Dart_Handle result;
  if (read_status == ZX_OK) {
    FX_DCHECK(actual_bytes <= size);

    // return a ReadResult object.
    result =
        ConstructDartObject(kReadResult, tonic::ToDart(read_status),
                            bytes.dart_handle(), tonic::ToDart(actual_bytes));
  } else {
    result = ConstructDartObject(kReadResult, tonic::ToDart(read_status));
  }
  bytes.Release();

  return result;
}

Dart_Handle System::socketWrite(std::shared_ptr<Handle> socket,
                                const tonic::DartByteData& data,
                                uint32_t options) {
  if (!socket || !socket->isValid()) {
    data.Release();
    return ConstructDartObject(kWriteResult, tonic::ToDart(ZX_ERR_BAD_HANDLE));
  }

  size_t actual;
  zx::unowned<zx::socket> zx_socket(socket->handle());
  zx_status_t write_status =
      zx_socket->write(options, data.data(), data.length_in_bytes(), &actual);

  Dart_Handle result;
  if (write_status == ZX_OK) {
    // return a WriteResult object.
    result = ConstructDartObject(kWriteResult, tonic::ToDart(write_status),
                                 tonic::ToDart(actual));
  } else {
    result = ConstructDartObject(kWriteResult, tonic::ToDart(write_status));
  }
  data.Release();

  return result;
}

Dart_Handle System::vmoCreate(size_t size, uint32_t options) {
  zx::vmo vmo;
  zx_status_t status = zx::vmo::create(size, options, &vmo);
  if (status != ZX_OK) {
    return ConstructDartObject(kHandleResult, tonic::ToDart(status));
  } else {
    return ConstructDartObject(kHandleResult, tonic::ToDart(status),
                               tonic::ToDart(Handle::Create(std::move(vmo))));
  }
}

Dart_Handle System::vmoFromFile(std::string path) {
  int fd = FdFromPath(path);
  if (fd == -1) {
    return ConstructDartObject(kFromFileResult, tonic::ToDart(ZX_ERR_IO));
  }

  struct stat stat_struct;
  if (fstat(fd, &stat_struct) == -1) {
    close(fd);
    return ConstructDartObject(kFromFileResult, tonic::ToDart(ZX_ERR_IO));
  }

  zx::vmo vmo;
  zx_status_t status = fdio_get_vmo_clone(fd, vmo.reset_and_get_address());
  if (status != ZX_OK) {
    close(fd);
    return ConstructDartObject(kFromFileResult, tonic::ToDart(status));
  }
  close(fd);

  return ConstructDartObject(kFromFileResult, tonic::ToDart(status),
                             tonic::ToDart(Handle::Create(std::move(vmo))),
                             tonic::ToDart(stat_struct.st_size));
}

Dart_Handle System::vmoGetSize(std::shared_ptr<Handle> vmo) {
  if (!vmo || !vmo->isValid()) {
    return ConstructDartObject(kGetSizeResult,
                               tonic::ToDart(ZX_ERR_BAD_HANDLE));
  }

  size_t size;
  zx::unowned<zx::vmo> zx_vmo(vmo->handle());
  zx_status_t status = zx_vmo->get_size(&size);

  return ConstructDartObject(kGetSizeResult, tonic::ToDart(status),
                             tonic::ToDart(size));
}

zx_status_t System::vmoSetSize(std::shared_ptr<Handle> vmo, size_t size) {
  if (!vmo || !vmo->isValid()) {
    return ZX_ERR_BAD_HANDLE;
  }

  zx::unowned<zx::vmo> zx_vmo(vmo->handle());
  return zx_vmo->set_size(size);
}

Dart_Handle System::vmoMap(std::shared_ptr<Handle> vmo) {
  if (!vmo || !vmo->isValid())
    return ConstructDartObject(kMapResult, tonic::ToDart(ZX_ERR_BAD_HANDLE));

  size_t size;
  zx::unowned<zx::vmo> zx_vmo(vmo->handle());
  zx_status_t status = zx_vmo->get_size(&size);
  if (status != ZX_OK)
    return ConstructDartObject(kMapResult, tonic::ToDart(status));

  uintptr_t mapped_addr;
  status = zx::vmar::root_self()->map(0, *zx_vmo, 0, size, ZX_VM_PERM_READ,
                                      &mapped_addr);
  if (status != ZX_OK)
    return ConstructDartObject(kMapResult, tonic::ToDart(status));

  void* data = reinterpret_cast<void*>(mapped_addr);
  Dart_Handle object = Dart_NewExternalTypedData(Dart_TypedData_kUint8, data,
                                                 static_cast<intptr_t>(size));
  FX_DCHECK(!tonic::LogIfError(object));

  SizedRegion* r = new SizedRegion(data, size);
  Dart_NewWeakPersistentHandle(object, reinterpret_cast<void*>(r),
                               static_cast<intptr_t>(size) + sizeof(*r),
                               VmoMapFinalizer);

  return ConstructDartObject(kMapResult, tonic::ToDart(ZX_OK), object);
}

Dart_Handle System::vmoRead(std::shared_ptr<Handle> vmo,
                            uint64_t offset,
                            size_t size) {
  if (!vmo || !vmo->isValid()) {
    return ConstructDartObject(kReadResult, tonic::ToDart(ZX_ERR_BAD_HANDLE));
  }

  // TODO: constrain size?
  ByteDataScope bytes(size);
  zx::unowned<zx::vmo> zx_vmo(vmo->handle());
  zx_status_t status = zx_vmo->read(bytes.data(), offset, size);

  Dart_Handle result;
  if (status == ZX_OK) {
    result = ConstructDartObject(kReadResult, tonic::ToDart(status),
                                 bytes.dart_handle(), tonic::ToDart(size));
  } else {
    result = ConstructDartObject(kReadResult, tonic::ToDart(status));
  }
  bytes.Release();

  return result;
}

zx_status_t System::vmoWrite(std::shared_ptr<Handle> vmo,
                             uint64_t offset,
                             const tonic::DartByteData& data) {
  if (!vmo || !vmo->isValid()) {
    data.Release();
    return ZX_ERR_BAD_HANDLE;
  }

  zx::unowned<zx::vmo> zx_vmo(vmo->handle());
  zx_status_t status =
      zx_vmo->write(data.data(), offset, data.length_in_bytes());
  data.Release();

  return status;
}

}  // namespace zircon::dart
