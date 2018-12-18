// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_RESOURCE_CONTEXT_MANAGER_H_
#define FLUTTER_LIB_UI_RESOURCE_CONTEXT_MANAGER_H_

#include "flutter/fml/memory/weak_ptr.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace blink {
// Interface for methods that manage the GrContext creation, access, and
// release. Meant to be implemented by the owner of the GLContext, e.g. the
// PlatformView. These methods may be called from a non-platform task runner.
class ResourceContextManager {
 public:
  virtual sk_sp<GrContext> CreateResourceContext() = 0;

  // In almost all situations, this should be prefered to
  // `CreateResourceContext`. This will get the currently applicable GrContext
  // on the calling thread for the current GrContext, if one is available.
  // Calling this will ensure that, in the event the GLContext has changed, the
  // correct GrContext for the current GLContext will be used. Unlike all other
  // methods on the platform view, this one may be called on a non-platform task
  // runner.
  virtual sk_sp<GrContext> GetOrCreateResourceContext() = 0;

  virtual fml::WeakPtr<GrContext> GetOrCreateWeakResourceContext() = 0;

  virtual void ReleaseResourceContext() const = 0;
};

}  // namespace blink

#endif // FLUTTER_LIB_UI_RESOURCE_CONTEXT_MANAGER_H_
