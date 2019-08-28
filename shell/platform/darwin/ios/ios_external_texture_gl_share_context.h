// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_IOS_EXTERNAL_TEXTURE_SHARE_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_IOS_EXTERNAL_TEXTURE_SHARE_CONTEXT_H_

#include "flutter/flow/texture.h"
#include "flutter/fml/platform/darwin/cf_utils.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterTexture.h"

namespace flutter {

class IOSExternalTextureShareContext : public flutter::Texture {
 public:
  IOSExternalTextureShareContext(int64_t textureId, NSObject<FlutterShareTexture>* externalTexture);

  ~IOSExternalTextureShareContext() override;

  // Called from GPU thread.
  void Paint(SkCanvas& canvas, const SkRect& bounds, bool freeze, GrContext* context) override;

  void OnGrContextCreated() override;

  void OnGrContextDestroyed() override;

  void MarkNewFrameAvailable() override;

 private:
  NSObject<FlutterShareTexture>* external_texture_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSExternalTextureShareContext);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_IOS_EXTERNAL_TEXTURE_SHARE_CONTEXT_H_
