// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "flutter/lib/ui/painting/fragment_shader_builder.h"

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

static void FragmentShaderBuilder_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&FragmentShaderBuilder::Create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, FragmentShaderBuilder);

#define FOR_EACH_BINDING(V) \
  V(FragmentShaderBuilder, init)   \
  V(FragmentShaderBuilder, build)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void FragmentShaderBuilder::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"FragmentShaderBuilder_constructor", FragmentShaderBuilder_constructor, 1, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

void FragmentShaderBuilder::init(std::string sksl, bool debugPrintSksl) {
  SkRuntimeEffect::Result result =
      SkRuntimeEffect::MakeForShader(SkString(sksl));
  runtime_effect_ = result.effect;

  if (runtime_effect_ == nullptr) {
    Dart_ThrowException(tonic::ToDart(
        std::string("Invalid SkSL:\n") + sksl.c_str() +
        std::string("\nSkSL Error:\n") + result.errorText.c_str()));
    return;
  }
  if (debugPrintSksl) {
    FML_DLOG(INFO) << std::string("debugPrintSksl:\n") + sksl.c_str();
  }
}

fml::RefPtr<FragmentShader> FragmentShaderBuilder::build(
    Dart_Handle shader, const tonic::Float32List& uniforms) {
  auto sk_shader = runtime_effect_->makeShader(
       SkData::MakeWithCopy(uniforms.data(),
                            uniforms.num_elements() * sizeof(float)),
       0, 0, nullptr, false);
  return FragmentShader::Create(shader, std::move(sk_shader));
}

fml::RefPtr<FragmentShaderBuilder> FragmentShaderBuilder::Create() {
  return fml::MakeRefCounted<FragmentShaderBuilder>();
}

FragmentShaderBuilder::FragmentShaderBuilder() = default;

FragmentShaderBuilder::~FragmentShaderBuilder() = default;

}  // namespace flutter
