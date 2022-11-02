// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_

#include <mutex>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/incoming_message_dispatcher.h"
#include "flutter/shell/platform/embedder/embedder.h"

// Structs backing the opaque references used in the C API.
//
// DO NOT ADD ANY NEW CODE HERE. These are legacy, and are being phased out
// in favor of objects that own and manage the relevant functionality.

namespace flutter {
struct FlutterWindowsEngine;
struct FlutterWindowsView;
}  // namespace flutter

// Wrapper to distinguish the view controller ref from the view ref given out
// in the C API.
struct FlutterDesktopViewControllerState {
  // The view that backs this state object.
  std::unique_ptr<flutter::FlutterWindowsView> view;
};

// Wrapper to distinguish the plugin registrar ref from the engine ref given out
// in the C API.
struct FlutterDesktopPluginRegistrar {
  // The engine that owns this state object.
  flutter::FlutterWindowsEngine* engine = nullptr;
};

// Wrapper to distinguish the messenger ref from the engine ref given out
// in the C API.
struct FlutterDesktopMessenger {
 public:
  FlutterDesktopMessenger() = default;

  /// Getter for the engine field.
  flutter::FlutterWindowsEngine* GetEngine() const { return engine; }

  /// Setter for the engine field.
  /// Thread-safe.
  void SetEngine(flutter::FlutterWindowsEngine* arg_engine) {
    std::scoped_lock lock(mutex_);
    engine = arg_engine;
  }

  /// Increments the reference count.
  ///
  /// Thread-safe.
  FlutterDesktopMessenger* AddRef() {
    ref_count_.fetch_add(1);
    return this;
  }

  /// Decrements the reference count and deletes the object if the count has
  /// gone to zero.
  ///
  /// Thread-safe.
  void Release() {
    int32_t old_count = ref_count_.fetch_sub(1);
    if (old_count <= 1) {
      delete this;
    }
  }

  /// Returns the mutex associated with the |FlutterDesktopMessenger|.
  ///
  /// This mutex is used to synchronize reading or writing state inside the
  /// |FlutterDesktopMessenger| (ie |engine|).
  std::mutex& GetMutex() { return mutex_; }

  FlutterDesktopMessenger(const FlutterDesktopMessenger& value) = delete;
  FlutterDesktopMessenger& operator=(const FlutterDesktopMessenger& value) =
      delete;

 private:
  // The engine that owns this state object.
  flutter::FlutterWindowsEngine* engine = nullptr;
  std::mutex mutex_;
  std::atomic<int32_t> ref_count_ = 0;
};

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOW_STATE_H_
