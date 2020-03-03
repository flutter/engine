// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_RESOURCE_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_RESOURCE_CONTEXT_H_

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/platform/darwin/ios/ios_render_target.h"
#include "flutter/shell/platform/darwin/ios/rendering_api_selection.h"
#include "third_party/skia/include/gpu/GrContext.h"

@class CALayer;

namespace flutter {

class IOSContext {
 public:
  static std::unique_ptr<IOSContext> Create(IOSRenderingAPI rendering_api);

  IOSContext();

  virtual ~IOSContext();

  virtual std::unique_ptr<IOSRenderTarget> CreateRenderTarget(
      fml::scoped_nsobject<CALayer> layer) = 0;

  virtual sk_sp<GrContext> CreateResourceContext() = 0;

  virtual bool MakeCurrent() = 0;

  virtual bool ResourceMakeCurrent() = 0;

  virtual bool ClearCurrent() = 0;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(IOSContext);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_RESOURCE_CONTEXT_H_
