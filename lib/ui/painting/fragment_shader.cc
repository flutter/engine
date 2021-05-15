// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/fragment_shader.h"

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/skia/include/core/SkString.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/typed_data/typed_list.h"

using tonic::ToDart;

namespace flutter {

static void FragmentShader_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&FragmentShader::Create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, FragmentShader);

#define FOR_EACH_BINDING(V)          \
  V(FragmentShader, init)   \
  V(FragmentShader, update)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

sk_sp<SkShader> FragmentShader::shader(SkSamplingOptions sampling) {
  return shader_;
}

void FragmentShader::init(std::string sksl) {
  SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(SkString(sksl));
  runtime_effect_ = result.effect;
  if (runtime_effect_ != nullptr) {
    Dart_Handle err = Dart_ThrowException(tonic::ToDart(
        std::string("Invalid SkSL:\n") + sksl.c_str() +
        std::string("\nSkSL Error:\n") + result.errorText.c_str()));
    if (err) {
      FML_DLOG(ERROR) << Dart_GetError(err);
    }
    return;
  }
}

void FragmentShader::update(const tonic::Float32List& uniforms, Dart_Handle children) {
  std::vector<Shader*> shaders =
      tonic::DartConverter<std::vector<Shader*>>::FromDart(children);
  std::vector<sk_sp<SkShader>> children_sk(shaders.size());
  for (size_t i = 0; i < shaders.size(); i++) {
    children_sk[i] = shaders[i]->shader(SkSamplingOptions(SkFilterQuality::kHigh_SkFilterQuality));
  }
  shader_ = runtime_effect_->makeShader(
      uniforms.num_elements() == 0
          ? SkData::MakeEmpty()
          : SkData::MakeWithCopy(uniforms.data(),
                                 uniforms.num_elements() * sizeof(float)),
      children_sk.data(), children_sk.size(), nullptr, false);
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
