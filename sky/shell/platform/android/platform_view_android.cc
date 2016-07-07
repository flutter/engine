// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform/android/platform_view_android.h"

#include <android/input.h>
#include <android/native_window_jni.h>

#include "base/bind.h"
#include "base/location.h"
#include "jni/FlutterView_jni.h"
#include "sky/engine/core/script/dart_service_isolate.h"
#include "sky/engine/wtf/MakeUnique.h"
#include "sky/shell/shell.h"
#include "sky/shell/shell_view.h"

namespace sky {
namespace shell {

class ScopedAndroidNativeWindow {
 public:
  explicit ScopedAndroidNativeWindow(ANativeWindow* window) : window_(window) {
    if (window_ != nullptr) {
      ANativeWindow_acquire(window_);
    }
  }

  ScopedAndroidNativeWindow(ScopedAndroidNativeWindow&& other)
      : window_(other.window_) {
    other.window_ = nullptr;
  }

  ~ScopedAndroidNativeWindow() {
    if (window_ != nullptr) {
      ANativeWindow_release(window_);
      window_ = nullptr;
    }
  }

 private:
  ANativeWindow* window_;

  DISALLOW_COPY_AND_ASSIGN(ScopedAndroidNativeWindow);
};

// class AndroidGLContext() {
//  public:
//   AndroidGLContext();

//   ~AndroidGLContext();

//  private:
//   DISALLOW_COPY_AND_ASSIGN(AndroidGLContext);
// }

static jlong Attach(JNIEnv* env, jclass clazz, jint skyEngineHandle) {
  ShellView* shell_view = new ShellView(Shell::Shared());
  auto view = static_cast<PlatformViewAndroid*>(shell_view->view());
  view->SetShellView(std::unique_ptr<ShellView>(shell_view));
  view->ConnectToEngine(mojo::InterfaceRequest<SkyEngine>(
      mojo::ScopedMessagePipeHandle(mojo::MessagePipeHandle(skyEngineHandle))));
  return reinterpret_cast<jlong>(shell_view->view());
}

jint GetObservatoryPort(JNIEnv* env, jclass clazz) {
  return blink::DartServiceIsolate::GetObservatoryPort();
}

// static
bool PlatformViewAndroid::Register(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

// Per platform implementation of PlatformView::Create
PlatformView* PlatformView::Create(const Config& config,
                                   SurfaceConfig surface_config) {
  return new PlatformViewAndroid(config, surface_config);
}

PlatformViewAndroid::PlatformViewAndroid(const Config& config,
                                         SurfaceConfig surface_config)
    : PlatformView(config, surface_config), weak_factory_(this) {}

PlatformViewAndroid::~PlatformViewAndroid() {
  weak_factory_.InvalidateWeakPtrs();
}

void PlatformViewAndroid::SetShellView(std::unique_ptr<ShellView> shell_view) {
  DCHECK(!shell_view_);
  shell_view_ = std::move(shell_view);
}

void PlatformViewAndroid::Detach(JNIEnv* env, jobject obj) {
  shell_view_.reset();
  // Note: |this| has been destroyed at this point.
}

void PlatformViewAndroid::SurfaceCreated(JNIEnv* env,
                                         jobject obj,
                                         jobject jsurface) {
  base::android::ScopedJavaLocalRef<jobject> protector(env, jsurface);
  // Note: This ensures that any local references used by
  // ANativeWindow_fromSurface are released immediately. This is needed as a
  // workaround for https://code.google.com/p/android/issues/detail?id=68174
  {
    base::android::ScopedJavaLocalFrame scoped_local_reference_frame(env);
    ANativeWindow* window = ANativeWindow_fromSurface(env, jsurface);
    window_ = WTF::MakeUnique<ScopedAndroidNativeWindow>(window);
    ANativeWindow_release(window);
  }

  NotifyCreated();
}

void PlatformViewAndroid::SurfaceDestroyed(JNIEnv* env, jobject obj) {
  window_ = nullptr;
  NotifyDestroyed();
}

base::WeakPtr<sky::shell::PlatformView> PlatformViewAndroid::GetWeakViewPtr() {
  return weak_factory_.GetWeakPtr();
}

uint64_t PlatformViewAndroid::DefaultFramebuffer() const {
  // FBO 0 is the default window bound framebuffer on Android.
  return 0;
}

bool PlatformViewAndroid::ContextMakeCurrent() {}

bool PlatformViewAndroid::SwapBuffers() {}

}  // namespace shell
}  // namespace sky
