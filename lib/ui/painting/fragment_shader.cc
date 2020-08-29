// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/fragment_shader.h"

#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/skia/include/core/SkString.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"

using tonic::ToDart;

namespace flutter {

static void FragmentShader_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&FragmentShader::Create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, FragmentShader);

#define FOR_EACH_BINDING(V) \
  V(FragmentShader, initWithSource) \
  V(FragmentShader, setTime) \
  V(FragmentShader, setImage)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void FragmentShader::initWithSource(const std::string& source) {
  initEffect(SkString(source.c_str()));
  setShader();
}

void FragmentShader::setTime(float time) {
  t_ = time;
  setShader();
}

void FragmentShader::setImage(CanvasImage* image,
                              SkTileMode tmx,
                              SkTileMode tmy,
                              const tonic::Float64List& matrix4) {
  SkMatrix sk_matrix = ToSkMatrix(matrix4);
  input_ = image->image()->makeShader(tmx, tmy, &sk_matrix);
  setShader();
}

void FragmentShader::initEffect(SkString sksl) {
  SkString err;
  std::tie(runtime_effect_, err) = SkRuntimeEffect::Make(sksl);
  if (!runtime_effect_) {
    FML_DLOG(ERROR) << "Invalid SKSL:\n" << sksl.c_str() << "\nSKSL Error:\n" << err.c_str();
  }
}

// Creates a builder and sets time uniform and image child if
// they are valid and are defined in the SKSL program.
//
// After any uniforms/children are set on the builser, the shader is 
// created and set.
void FragmentShader::setShader() {
  builder_ = std::make_unique<SkRuntimeShaderBuilder>(runtime_effect_);

  // Only update the time if the uniform is declared in the program.
  SkRuntimeShaderBuilder::BuilderUniform t_uniform = builder_->uniform("t");
  if (t_uniform.fVar != nullptr) {
    t_uniform = t_;
  }

  // Only update the input if the child is declared in the program.
  SkRuntimeShaderBuilder::BuilderChild input_child = builder_->child("input");
  if (input_child.fIndex != -1 && input_ != nullptr) {
    input_child = input_;
  }

  set_shader(UIDartState::CreateGPUObject(builder_->makeShader(nullptr, false)));
}

void FragmentShader::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"FragmentShader_constructor", FragmentShader_constructor, 1, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

fml::RefPtr<FragmentShader> FragmentShader::Create() {
  return fml::MakeRefCounted<FragmentShader>();
}

FragmentShader::FragmentShader() {}

FragmentShader::~FragmentShader() = default;

}  // namespace flutter

