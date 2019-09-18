// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_TEXTURE_SHARE_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_TEXTURE_SHARE_CONTEXT_H_

#include <GLES/gl.h>
#include "flutter/flow/texture.h"
#include "flutter/fml/platform/android/jni_weak_ref.h"

namespace flutter {

// This is another solution for Flutter's ExternalTexture.
// The original ExternalTexture uses SurfaceTexture to update the frame data
// that native video object produces to an OpenGL texture. In this scheme, we
// directly pass an OpenGL texture ID to the ExternalTexture object, and avoid
// the performance consumption of data writing to SurfaceTexture
class AndroidExternalTextureShareContext : public flutter::Texture {
 public:
  AndroidExternalTextureShareContext(int64_t id, int64_t shareTextureID);

  ~AndroidExternalTextureShareContext() override;

  void Paint(SkCanvas& canvas,
             const SkRect& bounds,
             bool freeze,
             GrContext* context) override;

  void OnGrContextCreated() override;

  void OnGrContextDestroyed() override;

  void MarkNewFrameAvailable() override;

 private:
  fml::jni::JavaObjectWeakGlobalRef surface_texture_;

  GLuint texture_id_ = 0;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidExternalTextureShareContext);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_EXTERNAL_TEXTURE_GL_H_
