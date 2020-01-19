// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_SESSION_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_SESSION_H_

#include <lib/async/dispatcher.h>
#include <lib/ui/scenic/cpp/session.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "flutter/shell/platform/fuchsia/flutter/renderer.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace flutter_runner {

// This embedder component is responsible for managing the Scenic |Session|,
// especially the complexities of the |Present| callback.
//
// This class is used from multiple threads and has specific threading
// requirements.
//  + AwaitPresent() will be called from the UI thread.
//  + QueuePresent should only be called from the raster thread.
//  + session() should only be called from the raster thread.
//  + `pending_present_baton_` is shared between the UI and raster threads, and
//  is
//    thus locked behind an atomic.
class ScenicSession final {
 public:
  using EventCallback =
      std::function<void(std::vector<fuchsia::ui::scenic::Event>)>;

  ScenicSession() = default;
  ScenicSession(const ScenicSession&) = delete;
  ScenicSession(ScenicSession&&) = delete;
  ~ScenicSession() = default;

  ScenicSession& operator=(const ScenicSession&) = delete;
  ScenicSession& operator=(ScenicSession&&) = delete;

  // Call this to determine if the underlying |scenic::Session| is connected.
  //
  // This method can be called from any thread.
  bool connected() const { return session_connected_; }

  // Call this to access the underlying |scenic::Session|.
  //
  // This method can be called from any thread.
  scenic::Session* get() {
    FX_DCHECK(connected());
    return &session_.value();
  }

  // Call this to create the internal |scenic::Session| by connecting to Scenic.
  //
  // The |scenic::Session| must not already be connected i.e. |connected()|
  // returns false.
  //
  // This method must be called on the raster thread.
  void Connect(const Renderer::Context& context,
               EventCallback session_event_callback);

  // Call this to request an asynchronous wait for the next "Vsync", which is
  // the completion of the next |Present|.  The Sesion will fire the VSync
  // callback at that time to requst the generation of a new frame.
  //
  // The |scenic::Session| must be connected.
  //
  // This method can be called on any thread.
  void AwaitVsync(intptr_t baton);

  // Call this to trigger the end of the "Vsync" wait, regardless of the status
  // of any |Present| callbacks.
  //
  // The |scenic::Session| must be connected.
  //
  // This method can be called on any thread.
  void TriggerVsyncImmediately();

  // Call this to request that a |Present| be performed as soon as possible.
  //
  // The |scenic::Session| must be connected.
  //
  // This method must be called on the raster thread.
  void QueuePresent();

 private:
  void Present();
  void FireVsyncCallback(fuchsia::images::PresentationInfo presentation_info,
                         uint64_t now);

  Renderer::GetCurrentTimeCallback get_current_time_callback_;
  Renderer::VsyncCallback vsync_callback_;

  std::optional<scenic::Session> session_;
  std::atomic<bool> session_connected_ = false;

  // Flag given to the embedder in order to correlate VSyncs with |AwaitVsync|
  // requests.  Contention between the UI and raster threads requires locking it
  // in an atomic.
  std::atomic<intptr_t> pending_present_baton_ = 0;

  std::atomic<bool> presentation_callback_pending_ = false;
  std::atomic<bool> present_session_pending_ = false;

  // A flow event trace id for following |Session::Present| calls into
  // Scenic.  This will be incremented each |Session::Present| call.  By
  // convention, the Scenic side will also contain its own trace id that
  // begins at 0, and is incremented each |Session::Present| call.
  uint64_t next_present_trace_id_ = 0;
  uint64_t next_present_session_trace_id_ = 0;
  uint64_t processed_present_session_trace_id_ = 0;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_SESSION_H_
