// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_TESTING_ANDROID_NATIVE_ACTIVITY_NATIVE_ACTIVITY_H_
#define FLUTTER_TESTING_ANDROID_NATIVE_ACTIVITY_NATIVE_ACTIVITY_H_

#include <android/native_activity.h>

#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"

namespace flutter {

class NativeActivity {
 public:
  virtual ~NativeActivity();

  void Terminate();

  virtual void OnStart();

  virtual void OnStop();

  virtual void OnPause();

  virtual void OnResume();

  virtual std::shared_ptr<fml::Mapping> OnSaveInstanceState();

  virtual void OnWindowFocusChanged(bool has_focus);

  virtual void OnNativeWindowCreated(ANativeWindow* window);

  virtual void OnNativeWindowResized(ANativeWindow* window);

  virtual void OnNativeWindowRedrawNeeded(ANativeWindow* window);

  virtual void OnNativeWindowDestroyed(ANativeWindow* window);

  virtual void OnInputQueueCreated(AInputQueue* queue);

  virtual void OnInputQueueDestroyed(AInputQueue* queue);

  virtual void OnConfigurationChanged();

  virtual void OnLowMemory();

 protected:
  explicit NativeActivity(ANativeActivity* activity);

 private:
  ANativeActivity* activity_ = nullptr;

  FML_DISALLOW_COPY_AND_ASSIGN(NativeActivity);
};

std::unique_ptr<NativeActivity> NativeActivityMain(
    ANativeActivity* activity,
    std::unique_ptr<fml::Mapping> saved_state);

}  // namespace flutter

#endif  // FLUTTER_TESTING_ANDROID_NATIVE_ACTIVITY_NATIVE_ACTIVITY_H_
