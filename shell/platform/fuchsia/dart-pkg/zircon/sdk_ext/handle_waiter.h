// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_HANDLE_WAITER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_HANDLE_WAITER_H_

#include <lib/async/cpp/wait.h>
#include <lib/zx/handle.h>
#include <zircon/syscalls.h>
#include <zircon/types.h>

#include <memory>

#include "flutter/third_party/tonic/dart_library_natives.h"
#include "flutter/third_party/tonic/dart_persistent_value.h"
#include "flutter/third_party/tonic/dart_wrappable.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace zircon::dart {

class Handle;

class HandleWaiter : public std::enable_shared_from_this<HandleWaiter>,
                     public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static void RegisterNatives(tonic::DartLibraryNatives* natives);

  static std::shared_ptr<HandleWaiter> Create(std::shared_ptr<Handle> handle,
                                              zx_signals_t signals,
                                              Dart_Handle callback);

  HandleWaiter(const HandleWaiter&) = delete;
  HandleWaiter(HandleWaiter&&) = delete;
  ~HandleWaiter();

  HandleWaiter& operator=(const HandleWaiter&) = delete;
  HandleWaiter& operator=(HandleWaiter&&) = delete;

  void cancel();

  bool is_pending() { return wait_.is_pending(); }

 private:
  explicit HandleWaiter(std::shared_ptr<Handle> handle,
                        zx_signals_t signals,
                        Dart_Handle callback);

  void OnWaitComplete(async_dispatcher_t* dispatcher,
                      async::WaitBase* wait,
                      zx_status_t status,
                      const zx_packet_signal_t* signal);

  // |DartWrappable|
  void RetainDartWrappableReference() const override {
    vm_reference_ = shared_from_this();
  }
  void ReleaseDartWrappableReference() const override { vm_reference_.reset(); }

  tonic::DartPersistentValue callback_;

  async::WaitMethod<HandleWaiter, &HandleWaiter::OnWaitComplete> wait_;

  mutable std::shared_ptr<const HandleWaiter> vm_reference_;

  std::weak_ptr<Handle> weak_handle_;
};

}  // namespace zircon::dart

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_ZIRCON_SDK_EXT_HANDLE_WAITER_H_
