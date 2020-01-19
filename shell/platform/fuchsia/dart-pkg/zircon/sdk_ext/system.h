// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_SYSTEM_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_SYSTEM_H_

#include <zircon/types.h>

#include <memory>
#include <string>
#include <vector>

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle.h"
#include "flutter/third_party/tonic/dart_library_natives.h"
#include "flutter/third_party/tonic/typed_data/dart_byte_data.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace zircon::dart {

class System : public std::enable_shared_from_this<System>,
               public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static void RegisterNatives(tonic::DartLibraryNatives* natives);

  static Dart_Handle channelCreate(uint32_t options);
  static Dart_Handle channelFromFile(std::string path);
  static zx_status_t channelWrite(std::shared_ptr<Handle> channel,
                                  const tonic::DartByteData& data,
                                  std::vector<std::shared_ptr<Handle>> handles);
  // TODO(ianloic): Add channelRead
  static Dart_Handle channelQueryAndRead(std::shared_ptr<Handle> channel);
  static zx_status_t connectToService(std::string path,
                                      std::shared_ptr<Handle> channel);

  static zx_time_t clockGet(zx_clock_t clock_id);

  static Dart_Handle eventpairCreate(uint32_t options);

  static Dart_Handle socketCreate(uint32_t options);
  static Dart_Handle socketRead(std::shared_ptr<Handle> socket, size_t size);
  static Dart_Handle socketWrite(std::shared_ptr<Handle> socket,
                                 const tonic::DartByteData& data,
                                 uint32_t options);

  static Dart_Handle vmoCreate(size_t size, uint32_t options);
  static Dart_Handle vmoFromFile(std::string path);
  static Dart_Handle vmoGetSize(std::shared_ptr<Handle> vmo);
  static zx_status_t vmoSetSize(std::shared_ptr<Handle> vmo, size_t size);
  static Dart_Handle vmoMap(std::shared_ptr<Handle> vmo);
  static Dart_Handle vmoRead(std::shared_ptr<Handle> vmo,
                             uint64_t offset,
                             size_t size);
  static zx_status_t vmoWrite(std::shared_ptr<Handle> vmo,
                              uint64_t offset,
                              const tonic::DartByteData& data);

 private:
  // |DartWrappable|
  void RetainDartWrappableReference() const override {
    vm_reference_ = shared_from_this();
  }
  void ReleaseDartWrappableReference() const override { vm_reference_.reset(); }

  mutable std::shared_ptr<const System> vm_reference_;
};

}  // namespace zircon::dart

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_SYSTEM_H_
