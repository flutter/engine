// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle_waiter.h"

#include <lib/async/default.h>

#include "flutter/shell/platform/fuchsia/dart-pkg/zircon/sdk_ext/handle.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/third_party/tonic/dart_binding_macros.h"

namespace zircon::dart {
namespace {

void InvokeCallback(tonic::DartPersistentValue callback,
                    zx_status_t status,
                    const zx_packet_signal_t* signal) {
  auto state = callback.dart_state().lock();
  FX_DCHECK(state.get());

  tonic::DartState::Scope scope(state);

  // Put the closure invocation on the microtask queue.
  Dart_Handle zircon_lib = Dart_LookupLibrary(tonic::ToDart("dart:zircon"));
  FX_DCHECK(!tonic::LogIfError(zircon_lib));

  Dart_Handle owc_type =
      Dart_GetClass(zircon_lib, tonic::ToDart("_OnWaitCompleteClosure"));
  FX_DCHECK(!tonic::LogIfError(owc_type));

  FX_DCHECK(!callback.is_empty());
  std::vector<Dart_Handle> owc_args{callback.Release(), tonic::ToDart(status),
                                    tonic::ToDart(signal->observed)};
  Dart_Handle owc =
      Dart_New(owc_type, Dart_Null(), owc_args.size(), owc_args.data());
  FX_DCHECK(!tonic::LogIfError(owc));

  Dart_Handle closure = Dart_GetField(owc, tonic::ToDart("_closure"));
  FX_DCHECK(!tonic::LogIfError(closure));

  // TODO(issue#tbd): Use tonic::DartMicrotaskQueue::ScheduleMicrotask()
  // instead when tonic::tonic::DartState gets a microtask queue field.
  Dart_Handle async_lib = Dart_LookupLibrary(tonic::ToDart("dart:async"));
  FX_DCHECK(!tonic::LogIfError(async_lib));
  std::vector<Dart_Handle> sm_args{closure};
  Dart_Handle sm_result =
      Dart_Invoke(async_lib, tonic::ToDart("scheduleMicrotask"), sm_args.size(),
                  sm_args.data());
  FX_DCHECK(!tonic::LogIfError(sm_result));
}

}  // namespace

IMPLEMENT_WRAPPERTYPEINFO(zircon, HandleWaiter);

#define FOR_EACH_BINDING(V) V(HandleWaiter, cancel)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void HandleWaiter::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

std::shared_ptr<HandleWaiter> HandleWaiter::Create(
    std::shared_ptr<Handle> handle,
    zx_signals_t signals,
    Dart_Handle callback) {
  return std::shared_ptr<HandleWaiter>(
      new HandleWaiter(handle, signals, callback));
}

HandleWaiter::HandleWaiter(std::shared_ptr<Handle> handle,
                           zx_signals_t signals,
                           Dart_Handle callback)
    : callback_(tonic::DartState::Current(), callback),
      wait_(this, handle->handle(), signals),
      weak_handle_(handle) {
  FX_CHECK(handle->isValid());

  zx_status_t status = wait_.Begin(async_get_default_dispatcher());
  FX_DCHECK(status == ZX_OK);
}

HandleWaiter::~HandleWaiter() {
  cancel();
}

void HandleWaiter::cancel() {
  auto locked_handle = weak_handle_.lock();
  if (locked_handle) {
    FX_DCHECK(locked_handle->isValid());
    FX_DCHECK(wait_.is_pending());

    // Cancel the wait.
    wait_.Cancel();
    weak_handle_.reset();
    FX_DCHECK(!wait_.is_pending());

    // Remove this waiter from the handle.
    // WARNING: Might delete this.
    locked_handle->ReleaseWaiter(shared_from_this());
  }
}

void HandleWaiter::OnWaitComplete(async_dispatcher_t* dispatcher,
                                  async::WaitBase* wait,
                                  zx_status_t status,
                                  const zx_packet_signal_t* signal) {
  auto locked_handle = weak_handle_.lock();
  if (locked_handle) {
    FX_DCHECK(locked_handle->isValid());
    FX_DCHECK(!callback_.is_empty());

    InvokeCallback(std::move(callback_), status, signal);
    weak_handle_.reset();

    // Remove this waiter from the handle.
    // WARNING: Might delete this.
    locked_handle->ReleaseWaiter(shared_from_this());
  }
}

}  // namespace zircon::dart
