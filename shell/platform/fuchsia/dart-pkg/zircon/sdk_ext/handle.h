// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_HANDLE_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_HANDLE_H_

#include <lib/zx/handle.h>
#include <zircon/syscalls.h>
#include <zircon/types.h>

#include <memory>
#include <vector>

#include "flutter/third_party/tonic/converter/dart_converter.h"
#include "flutter/third_party/tonic/dart_library_natives.h"
#include "flutter/third_party/tonic/dart_persistent_value.h"
#include "flutter/third_party/tonic/dart_wrappable.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace zircon::dart {

class HandleWaiter;

// Handle is the native peer of a Dart object (Handle in dart:zircon)
// that holds an zx_handle_t. It tracks active waiters on handle too.
class Handle : public std::enable_shared_from_this<Handle>,
               public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static void RegisterNatives(tonic::DartLibraryNatives* natives);

  static std::shared_ptr<Handle> createInvalid();

  static std::shared_ptr<Handle> Create(zx_handle_t handle);
  static std::shared_ptr<Handle> Create(zx::handle handle);
  static std::shared_ptr<Handle> Unwrap(Dart_Handle handle);

  Handle(const Handle&) = delete;
  Handle(Handle&&) = delete;
  ~Handle();

  Handle& operator=(const Handle&) = delete;
  Handle& operator=(Handle&&) = delete;

  zx_handle_t handle() const { return handle_.get(); }
  bool isValid() const { return handle_.is_valid(); }

  zx_status_t close();

  std::shared_ptr<Handle> duplicate(zx_rights_t rights);
  std::shared_ptr<HandleWaiter> asyncWait(zx_signals_t signals,
                                          Dart_Handle callback);

  zx::handle ReleaseHandle();

  void ReleaseWaiter(std::shared_ptr<HandleWaiter> waiter);

 private:
  explicit Handle(zx::handle handle);

  // |DartWrappable|
  void RetainDartWrappableReference() const override {
    vm_reference_ = shared_from_this();
  }
  void ReleaseDartWrappableReference() const override { vm_reference_.reset(); }

  // Some cached persistent handles so handle wait completers are faster.
  tonic::DartPersistentValue on_wait_completer_type_;
  tonic::DartPersistentValue async_lib_;
  tonic::DartPersistentValue closure_string_;
  tonic::DartPersistentValue schedule_microtask_string_;

  std::vector<std::shared_ptr<HandleWaiter>> waiters_;

  mutable std::shared_ptr<const Handle> vm_reference_;

  zx::handle handle_;
};

}  // namespace zircon::dart

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_HANDLE_H_
