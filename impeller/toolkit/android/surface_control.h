// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_ANDROID_SURFACE_CONTROL_H_
#define FLUTTER_IMPELLER_TOOLKIT_ANDROID_SURFACE_CONTROL_H_

#include "flutter/fml/unique_object.h"
#include "impeller/toolkit/android/proc_table.h"

namespace impeller::android {

//------------------------------------------------------------------------------
/// @brief      A wrapper for ASurfaceControl.
///             https://developer.android.com/ndk/reference/group/native-activity#asurfacecontrol
///
///             Instances of this class represent a node in the hierarchy of
///             surfaces sent to the system compositor for final composition.
///
///             This wrapper is only available on Android API 29 and above.
///
class SurfaceControl {
 public:
  //----------------------------------------------------------------------------
  /// @brief      Creates a new surface control and adds it as a child of the
  ///             given window.
  ///
  /// @param      window      The window
  /// @param[in]  debug_name  A debug name. See it using
  ///                         `adb shell dumpsys SurfaceFlinger` along with
  ///                         other control properties. If no debug name is
  ///                         specified, the value "Impeller Layer" is used.
  ///
  SurfaceControl(ANativeWindow* window, const char* debug_name = nullptr);

  ~SurfaceControl();

  SurfaceControl(const SurfaceControl&) = delete;

  SurfaceControl& operator=(const SurfaceControl&) = delete;

  bool IsValid() const;

  ASurfaceControl* GetHandle() const;

 private:
  struct UniqueASurfaceControlTraits {
    static ASurfaceControl* InvalidValue() { return nullptr; }

    static bool IsValid(ASurfaceControl* value) {
      return value != InvalidValue();
    }

    static void Free(ASurfaceControl* value) {
      GetProcTable().ASurfaceControl_release(value);
    }
  };

  fml::UniqueObject<ASurfaceControl*, UniqueASurfaceControlTraits> control_;
};

}  // namespace impeller::android

#endif  // FLUTTER_IMPELLER_TOOLKIT_ANDROID_SURFACE_CONTROL_H_
