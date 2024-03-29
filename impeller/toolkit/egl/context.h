// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_EGL_CONTEXT_H_
#define FLUTTER_IMPELLER_TOOLKIT_EGL_CONTEXT_H_

#include <functional>

#include "flutter/fml/macros.h"
#include "impeller/base/comparable.h"
#include "impeller/base/thread.h"
#include "impeller/toolkit/egl/egl.h"

namespace impeller {
namespace egl {

class Surface;

class Context {
 public:
  Context(EGLDisplay display, EGLContext context);

  ~Context();

  bool IsValid() const;

  const EGLContext& GetHandle() const;

  bool MakeCurrent(const Surface& surface) const;

  bool ClearCurrent() const;

  enum class LifecycleEvent {
    kDidMakeCurrent,
    kWillClearCurrent,
  };
  using LifecycleListener = std::function<void(LifecycleEvent)>;
  std::optional<UniqueID> AddLifecycleListener(
      const LifecycleListener& listener);

  bool RemoveLifecycleListener(UniqueID id);

 private:
  EGLDisplay display_ = EGL_NO_DISPLAY;
  EGLContext context_ = EGL_NO_CONTEXT;
  mutable RWMutex listeners_mutex_;
  std::map<UniqueID, LifecycleListener> listeners_ IPLR_GUARDED_BY(
      listeners_mutex_);

  void DispatchLifecyleEvent(LifecycleEvent event) const;

  Context(const Context&) = delete;

  Context& operator=(const Context&) = delete;
};

}  // namespace egl
}  // namespace impeller

#endif  // FLUTTER_IMPELLER_TOOLKIT_EGL_CONTEXT_H_
