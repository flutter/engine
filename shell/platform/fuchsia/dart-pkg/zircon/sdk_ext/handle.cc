// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle.h"

#include <algorithm>

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle_waiter.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/dart_binding_macros.h"

namespace zircon::dart {

IMPLEMENT_WRAPPERTYPEINFO(zircon, Handle);

#define FOR_EACH_STATIC_BINDING(V) V(Handle, createInvalid)

#define FOR_EACH_BINDING(V) \
  V(Handle, handle)         \
  V(Handle, isValid)        \
  V(Handle, close)          \
  V(Handle, duplicate)      \
  V(Handle, asyncWait)

FOR_EACH_STATIC_BINDING(DART_NATIVE_CALLBACK_STATIC)
FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void Handle::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({FOR_EACH_STATIC_BINDING(DART_REGISTER_NATIVE_STATIC)
                         FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

std::shared_ptr<Handle> Handle::createInvalid() {
  return Create(zx::handle());
}

std::shared_ptr<Handle> Handle::Create(zx_handle_t handle) {
  return Create(zx::handle(handle));
}

std::shared_ptr<Handle> Handle::Create(zx::handle handle) {
  return std::shared_ptr<Handle>(new Handle(std::move(handle)));
}

std::shared_ptr<Handle> Handle::Unwrap(Dart_Handle handle) {
  auto* unwrapped = tonic::DartConverter<Handle*>::FromDart(handle);
  return unwrapped->shared_from_this();
}

Handle::Handle(zx::handle handle) : handle_(std::move(handle)) {
  tonic::DartState* state = tonic::DartState::Current();
  FX_DCHECK(state);
  Dart_Handle zircon_lib = Dart_LookupLibrary(tonic::ToDart("dart:zircon"));
  FX_DCHECK(!tonic::LogIfError(zircon_lib));

  Dart_Handle on_wait_completer_type =
      Dart_GetClass(zircon_lib, tonic::ToDart("_OnWaitCompleteClosure"));
  FX_DCHECK(!tonic::LogIfError(on_wait_completer_type));
  on_wait_completer_type_.Set(state, on_wait_completer_type);

  Dart_Handle async_lib = Dart_LookupLibrary(tonic::ToDart("dart:async"));
  FX_DCHECK(!tonic::LogIfError(async_lib));
  async_lib_.Set(state, async_lib);

  Dart_Handle closure_string = tonic::ToDart("_closure");
  FX_DCHECK(!tonic::LogIfError(closure_string));
  closure_string_.Set(state, closure_string);
}

Handle::~Handle() {
  if (isValid()) {
    zx_status_t status = close();
    FX_DCHECK(status == ZX_OK);
  }
}

zx_status_t Handle::close() {
  handle_.reset();
  return ZX_OK;
}

std::shared_ptr<Handle> Handle::duplicate(zx_rights_t rights) {
  if (!isValid()) {
    return createInvalid();
  }

  zx::handle out_handle;
  zx_status_t status = handle_.duplicate(rights, &out_handle);
  if (status != ZX_OK) {
    return createInvalid();
  }
  return Create(std::move(out_handle));
}

std::shared_ptr<HandleWaiter> Handle::asyncWait(zx_signals_t signals,
                                                Dart_Handle callback) {
  if (!isValid()) {
    FX_LOG(WARNING) << "Attempt to wait on an invalid handle.";
    return nullptr;
  }

  std::shared_ptr<HandleWaiter> waiter =
      HandleWaiter::Create(shared_from_this(), signals, callback);
  waiters_.push_back(waiter);

  return waiter;
}

zx::handle Handle::ReleaseHandle() {
  FX_DCHECK(isValid());

  zx::handle handle;
  std::swap(handle, handle_);

  while (waiters_.size()) {
    // HandleWaiter::Cancel calls Handle::ReleaseWaiter which removes the
    // HandleWaiter from waiters_.
    FX_DCHECK(waiters_.back()->is_pending());
    waiters_.back()->cancel();
  }

  FX_DCHECK(!isValid());

  return handle;
}

void Handle::ReleaseWaiter(std::shared_ptr<HandleWaiter> waiter) {
  FX_DCHECK(waiter.get());

  auto iter = std::find(waiters_.begin(), waiters_.end(), waiter);
  FX_DCHECK(iter != waiters_.end());
  FX_DCHECK(*iter == waiter);

  waiters_.erase(iter);
}

}  // namespace zircon::dart
