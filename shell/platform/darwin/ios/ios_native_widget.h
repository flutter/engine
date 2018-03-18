// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_IOS_NATIVE_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_IOS_NATIVE_VIEW_H_

#import <UIKit/UIKit.h>
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/texture.h"
#include "flutter/fml/platform/darwin/cf_utils.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterTexture.h"

namespace shell {

class IOSNativeWidget : public flow::Texture {
 public:
  IOSNativeWidget(int64_t textureId, UIView* view);

  ~IOSNativeWidget() override;

  void UpdateScene(flow::SystemCompositorContext* context,
                   const SkRect& bounds) override;

  // Called from GPU thread.
  virtual void Paint(SkCanvas& canvas, const SkRect& bounds) override;

  void OnGrContextCreated() override;

  void OnGrContextDestroyed() override;

  UIView* view() { return external_view_; }

 private:
  UIView* external_view_;
  fml::CFRef<CVOpenGLESTextureCacheRef> cache_ref_;
  fml::CFRef<CVOpenGLESTextureRef> texture_ref_;
  FXL_DISALLOW_COPY_AND_ASSIGN(IOSNativeWidget);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_IOS_NATIVE_VIEW_H_
