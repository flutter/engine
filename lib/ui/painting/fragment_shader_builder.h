// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_FRAGMENT_SHADER_BUILDER_H_
#define FLUTTER_LIB_UI_PAINTING_FRAGMENT_SHADER_BUILDER_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/fragment_shader.h"
#include "third_party/skia/include/effects/SkRuntimeEffect.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/typed_data/typed_list.h"

#include <string>
#include <vector>

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace flutter {

class FragmentShaderBuilder
    : public RefCountedDartWrappable<FragmentShaderBuilder> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(FragmentShaderBuilder);

 public:
  ~FragmentShaderBuilder() override;
  static fml::RefPtr<FragmentShaderBuilder> Create();

  void init(std::string sksl, bool debugPrintSksl);

  fml::RefPtr<FragmentShader> build(Dart_Handle shader,
                                    const tonic::Float32List& uniforms);

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  FragmentShaderBuilder();
  sk_sp<SkRuntimeEffect> runtime_effect_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_FRAGMENT_SHADER_BUILDER_H_
