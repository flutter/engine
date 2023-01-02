// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_views.h"

#include <dcomp.h>
#include <stdint.h>
#include <mutex>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/shell/platform/embedder/embedder_struct_macros.h"

namespace flutter {

PlatformViewWindows::PlatformViewWindows(IDCompositionVisual* visual,
                           HWND window,
                           UpdateVisibleCb&& update_visible,
                           UpdateHiddenCb&& update_hidden)
    : window_(window),
      last_window_size_({0, 0}),
      last_window_pos_({0, 0}),
      visible_(false),
      update_visible_user_cb_(std::move(update_visible)),
      update_hidden_user_cb_(std::move(update_hidden)) {
  visual_.retain(visual);
}

PlatformViewWindows::~PlatformViewWindows() {}

void PlatformViewWindows::UpdateVisible(
    HDWP& winPosInfo,
    HWND& previous,
    const FlutterDesktopPlatformViewUpdate& update) {
  // TODO: Implement window parenting, DWM cloaking, and position updates

  if (window_) {
    if (update.is_simple_translate) {
      std::pair<uint32_t, uint32_t> size = {std::round(update.size.width), std::round(update.size.height)};
      std::pair<uint32_t, uint32_t> pos = {std::floor(update.transform.transX), std::floor(update.transform.transY)};

      UINT flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER;
      bool needs_update = false;

      if (size != last_window_size_) {
        last_window_size_ = size;
        FML_LOG(ERROR) << "New window size: " << size.first << ", " << size.second;
        needs_update = true;
      } else {
        flags |= SWP_NOSIZE;
      }

      if (pos != last_window_pos_) {
        last_window_pos_ = pos;
        needs_update = true;
      } else {
        flags |= SWP_NOMOVE;
      }

      // TODO: Don't compare HWND, instead compare platform view ID (64-bit) since they won't be reused.
      // NOTE: This requires changing platform view IDs to monotonically increasing numbers.
      if (needs_update || last_window_sibling_ != previous) {
        if (winPosInfo == 0) {
          winPosInfo = BeginDeferWindowPos(1);
        }

        winPosInfo = DeferWindowPos(
          winPosInfo,
          window_,
          previous,
          pos.first, pos.second,
          size.first, size.second,
          flags
        );

        last_window_sibling_ = previous;
      }

      previous = window_;
    } else {
      // TODO: Disable window
    }
  }

  if (update_visible_user_cb_) {
    update_visible_user_cb_(&update);
  }
}

void PlatformViewWindows::UpdateHidden() {
  // TODO: Implement window hiding

  if (update_hidden_user_cb_) {
    update_hidden_user_cb_();
  }
}

PlatformViewRegistrar::PlatformViewRegistrar(IDCompositionDevice* device) {
  device_.retain(device);
}

PlatformViewRegistrar::~PlatformViewRegistrar() {}

int64_t PlatformViewRegistrar::Register(
    const FlutterDesktopPlatformView* platform_view) {
  auto visual = SAFE_ACCESS(platform_view, visual, nullptr);
  auto window = SAFE_ACCESS(platform_view, window, nullptr);
  auto user_data = SAFE_ACCESS(platform_view, user_data, nullptr);
  auto update_visible_callback =
      SAFE_ACCESS(platform_view, update_visible_callback, nullptr);
  auto update_hidden_callback =
      SAFE_ACCESS(platform_view, update_hidden_callback, nullptr);

  FML_CHECK(visual != nullptr);

  PlatformViewWindows::UpdateVisibleCb update_visible {};
  PlatformViewWindows::UpdateHiddenCb update_hidden {};

  if (update_visible_callback) {
    update_visible = [=](const FlutterDesktopPlatformViewUpdate* update) {
      update_visible_callback(user_data, update);
    };
  }

  if (update_hidden_callback) {
    update_hidden = [=]() { update_hidden_callback(user_data); };
  }

  auto view = std::make_unique<PlatformViewWindows>(
      visual, window, std::move(update_visible), std::move(update_hidden));

  // TODO: Make platform view IDs monotonically increasing instead of using this.
  int64_t id = view->view_id();

  {
    std::unique_lock<std::mutex> lock(view_mutex_);
    FML_LOG(ERROR) << "Created view " << id;
    views_.emplace(id, std::move(view));
  }

  return id;
}

void PlatformViewRegistrar::Unregister(int64_t view_id) {
  {
    std::unique_lock<std::mutex> lock(view_mutex_);
    FML_LOG(ERROR) << "Remove view " << view_id;
    views_.erase(view_id);
  }
}

PlatformViewRegistrar::Lock::Lock(std::mutex& mutex) : lock_(mutex) {}

PlatformViewRegistrar::Lock PlatformViewRegistrar::AcquireLock() {
  return Lock(view_mutex_);
}

PlatformViewWindows* PlatformViewRegistrar::FindViewById(const Lock& lock,
                                                  int64_t view_id) {
  auto found = views_.find(view_id);
  if (found != views_.end()) {
    return found->second.get();
  } else {
    return nullptr;
  }
}

}  // namespace flutter
