// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_FRAGMENT_SHADER_H_
#define FLUTTER_LIB_UI_PAINTING_FRAGMENT_SHADER_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/lib/ui/painting/image_shader.h"
#include "flutter/lib/ui/painting/shader.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/skia/include/effects/SkRuntimeEffect.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/tonic/typed_data/typed_list.h"

#include <string>
#include <vector>

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace flutter {

class FragmentShader : public Shader {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(FragmentShader);

 public:
  ~FragmentShader() override;
  static fml::RefPtr<FragmentShader> Create();

  void initWithSource(const std::string& source);
  void initWithSPIRV(const tonic::Uint8List& data);

  void setTime(float time);

  void setImage(CanvasImage* image,
                SkTileMode tmx,
                SkTileMode tmy,
                const tonic::Float64List& matrix4);

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  FragmentShader();

  void setShader();
  void initEffect(SkString sksl);

  // Variables that require setShader() to be called when changed.
  float t_;
  sk_sp<SkShader> input_;

  // Since the sksl cannot be updated, the effect can be
  // created once and re-used.
  sk_sp<SkRuntimeEffect> runtime_effect_;

  std::unique_ptr<SkRuntimeShaderBuilder> builder_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_FRAGMENT_SHADER_H_
