// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEWS_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEWS_H_

#include <dcomp.h>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/platform_views.h"
#include "flutter/shell/platform/windows/public/flutter_windows_platform_view.h"
#include "third_party/skia/include/gpu/d3d/GrD3DTypes.h"

namespace flutter {

class PlatformViewWindows {
 public:
  using UpdateVisibleCb =
      std::function<void(const FlutterDesktopPlatformViewUpdate*)>;
  using UpdateHiddenCb = std::function<void()>;

  explicit PlatformViewWindows(IDCompositionVisual* visual,
                        HWND window,
                        UpdateVisibleCb&& update_visible,
                        UpdateHiddenCb&& update_hidden);
  ~PlatformViewWindows();

  void UpdateVisible(HDWP& winPosInfo,
                     HWND& previous,
                     const FlutterDesktopPlatformViewUpdate& update);
  void UpdateHidden();

  int64_t view_id() const { return reinterpret_cast<int64_t>(this); }

  HWND window() const { return window_; }

  IDCompositionVisual* visual() const { return visual_.get(); }

 private:
  // Owned by the embedder or plugin.
  gr_cp<IDCompositionVisual> visual_;
  // For accepting input events from the kernel.
  // May be null.
  HWND window_;
  // Size of the platform view the last frame it was visible.
  // Used to avoid constantly resizing platform views.
  std::pair<uint32_t, uint32_t> last_window_size_;
  std::pair<uint32_t, uint32_t> last_window_pos_;
  HWND last_window_sibling_;
  // Whether the platform view is in the visible state.
  bool visible_;
  // User callback called during UpdateVisible.
  UpdateVisibleCb update_visible_user_cb_;
  // User callback called during UpdateHidden.
  UpdateHiddenCb update_hidden_user_cb_;
};

class PlatformViewRegistrar {
 public:
  explicit PlatformViewRegistrar(IDCompositionDevice* device);
  ~PlatformViewRegistrar();

  int64_t Register(const FlutterDesktopPlatformView* platform_view);
  void Unregister(int64_t view_id);

  IDCompositionDevice* device() const { return device_.get(); }

  class Lock {
   public:
    ~Lock() = default;

   private:
    friend class PlatformViewRegistrar;
    explicit Lock(std::mutex&);
    std::unique_lock<std::mutex> lock_;
  };

  Lock AcquireLock();
  PlatformViewWindows* FindViewById(const Lock& lock, int64_t view_id);

 private:
  gr_cp<IDCompositionDevice> device_;
  std::unordered_map<int64_t, std::unique_ptr<PlatformViewWindows>> views_;
  std::mutex view_mutex_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEWS_H_
