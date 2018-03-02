// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_IOS_EXTERNAL_TEXTURE_LAYER_H_
#define FLUTTER_SHELL_PLATFORM_IOS_EXTERNAL_TEXTURE_LAYER_H_

#import <UIKit/UIKit.h>
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/texture.h"
#include "flutter/fml/platform/darwin/cf_utils.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterTexture.h"

namespace shell {

class IOSExternalTextureLayer : public flow::Texture {
 public:
  IOSExternalTextureLayer(int64_t textureId, UIView* externalLayer);

  ~IOSExternalTextureLayer() override;

  void UpdateScene(flow::SystemCompositorContext* context,
                   const SkRect& bounds) override;

  // Called from GPU thread.
  void Paint(flow::Layer::PaintContext context, const SkRect& bounds) override;

  void OnGrContextCreated() override;

  void OnGrContextDestroyed() override;

  bool NeedsSystemComposite() override;

  UIView* layer() { return external_layer_; }

 private:
  UIView* external_layer_;
  fml::CFRef<CVOpenGLESTextureCacheRef> cache_ref_;
  fml::CFRef<CVOpenGLESTextureRef> texture_ref_;
  FXL_DISALLOW_COPY_AND_ASSIGN(IOSExternalTextureLayer);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_IOS_EXTERNAL_TEXTURE_LAYER_H_
