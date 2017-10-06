// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_PLATFORM_SURFACE_H_
#define FLUTTER_FLOW_PLATFORM_SURFACE_H_

#include <map>
#include "flutter/common/threads.h"
#include "lib/fxl/synchronization/waitable_event.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/GrTexture.h"
#include "third_party/skia/include/gpu/GrTypes.h"

namespace flow {

class PlatformSurface {
  friend class PlatformSurfaceRegistry;

 public:
  // Called from GPU thread.
  virtual ~PlatformSurface();

  // Called from GPU thread.
  virtual sk_sp<SkImage> MakeSkImage(int width,
                                     int height,
                                     GrContext* grContext) = 0;

  // Called from GPU thread.
  virtual void OnGrContextCreated() = 0;

  // Called from GPU thread.
  virtual void OnGrContextDestroyed() = 0;

  size_t Id() { return id_; }

 private:
  size_t id_;
};

class PlatformSurfaceRegistry {
 public:
  PlatformSurfaceRegistry();
  ~PlatformSurfaceRegistry();

  // Called from GPU thread.
  size_t RegisterPlatformSurface(std::shared_ptr<PlatformSurface> surface);

  // Called from GPU thread.
  void UnregisterPlatformSurface(size_t id);

  // Called from GPU thread.
  std::shared_ptr<PlatformSurface> GetPlatformSurface(size_t id);

  // Called from GPU thread.
  void OnGrContextCreated();

  // Called from GPU thread.
  void OnGrContextDestroyed();

 private:
  std::map<size_t, std::shared_ptr<PlatformSurface>> mapping_;
  size_t counter_ = 0;
};

}  // namespace flow

#endif  // FLUTTER_FLOW_PLATFORM_SURFACE_H_
