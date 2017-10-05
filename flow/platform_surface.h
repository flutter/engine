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
  virtual ~PlatformSurface();

  // Called from GPU thread.
  virtual sk_sp<SkImage> MakeSkImage(int width,
                                     int height,
                                     GrContext* grContext) = 0;

  virtual void Attach() = 0;

  virtual void Detach() = 0;

  int Id() { return id_; }

 private:
  int id_;
};

class PlatformSurfaceRegistry {
 public:
  PlatformSurfaceRegistry();
  ~PlatformSurfaceRegistry();

  int RegisterPlatformSurface(PlatformSurface* surface);
  void DisposePlatformSurface(int id);
  PlatformSurface* GetPlatformSurface(int id);
  void AttachAll();
  void DetachAll();

 private:
  std::map<int, PlatformSurface*> mapping_;
};

}  // namespace flow

#endif  // FLUTTER_FLOW_PLATFORM_SURFACE_H_
