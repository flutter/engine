// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"
#include "wrappers.h"

using namespace Skwasm;

SKWASM_EXPORT Shader* shader_createLinearGradient(
    Point* endPoints,  // Two points
    Color* colors,
    Scalar* stops,
    int count,  // Number of stops/colors
    TileMode tileMode,
    Scalar* matrix33  // Can be nullptr
) {
  if (matrix33) {
    Matrix localMatrix = createMatrix(matrix33);
    return GradientShader::MakeLinear(endPoints, colors, stops, count, tileMode,
                                      0, &localMatrix)
        .release();
  } else {
    return GradientShader::MakeLinear(endPoints, colors, stops, count, tileMode)
        .release();
  }
}

SKWASM_EXPORT Shader* shader_createRadialGradient(Scalar centerX,
                                                  Scalar centerY,
                                                  Scalar radius,
                                                  Color* colors,
                                                  Scalar* stops,
                                                  int count,
                                                  TileMode tileMode,
                                                  Scalar* matrix33) {
  if (matrix33) {
    Matrix localMatrix = createMatrix(matrix33);
    return GradientShader::MakeRadial({centerX, centerY}, radius, colors, stops,
                                      count, tileMode, 0, &localMatrix)
        .release();
  } else {
    return GradientShader::MakeRadial({centerX, centerY}, radius, colors, stops,
                                      count, tileMode)
        .release();
  }
}

SKWASM_EXPORT Shader* shader_createConicalGradient(
    Point* endPoints,  // Two points
    Scalar startRadius,
    Scalar endRadius,
    Color* colors,
    Scalar* stops,
    int count,
    TileMode tileMode,
    Scalar* matrix33) {
  if (matrix33) {
    Matrix localMatrix = createMatrix(matrix33);
    return GradientShader::MakeTwoPointConical(
               endPoints[0], startRadius, endPoints[1], endRadius, colors,
               stops, count, tileMode, 0, &localMatrix)
        .release();

  } else {
    return GradientShader::MakeTwoPointConical(endPoints[0], startRadius,
                                               endPoints[1], endRadius, colors,
                                               stops, count, tileMode)
        .release();
  }
}

SKWASM_EXPORT Shader* shader_createSweepGradient(Scalar centerX,
                                                 Scalar centerY,
                                                 Color* colors,
                                                 Scalar* stops,
                                                 int count,
                                                 TileMode tileMode,
                                                 Scalar startAngle,
                                                 Scalar endAngle,
                                                 Scalar* matrix33) {
  if (matrix33) {
    Matrix localMatrix = createMatrix(matrix33);
    return GradientShader::MakeSweep(centerX, centerY, colors, stops, count,
                                     tileMode, startAngle, endAngle, 0,
                                     &localMatrix)
        .release();
  } else {
    return GradientShader::MakeSweep(centerX, centerY, colors, stops, count,
                                     tileMode, startAngle, endAngle, 0, nullptr)
        .release();
  }
}

SKWASM_EXPORT void shader_dispose(Shader* shader) {
  shader->unref();
}

SKWASM_EXPORT RuntimeEffect* runtimeEffect_create(SkString* source) {
  auto result = RuntimeEffect::MakeForShader(*source);
  if (result.effect == nullptr) {
    printf("Failed to compile shader. Error text:\n%s",
           result.errorText.data());
    return nullptr;
  } else {
    return result.effect.release();
  }
}

SKWASM_EXPORT void runtimeEffect_dispose(RuntimeEffect* effect) {
  effect->unref();
}

SKWASM_EXPORT size_t runtimeEffect_getUniformSize(RuntimeEffect* effect) {
  return effect->uniformSize();
}

SKWASM_EXPORT Shader* shader_createRuntimeEffectShader(
    RuntimeEffect* runtimeEffect,
    SkData* uniforms,
    Shader** children,
    size_t childCount) {
  std::vector<sk_sp<Shader>> childPointers;
  for (size_t i = 0; i < childCount; i++) {
    childPointers.emplace_back(sk_ref_sp<Shader>(children[i]));
  }
  return runtimeEffect
      ->makeShader(SkData::MakeWithCopy(uniforms->data(), uniforms->size()),
                   childPointers.data(), childCount, nullptr)
      .release();
}

SKWASM_EXPORT Shader* shader_createFromImage(Image* image,
                                             TileMode tileModeX,
                                             TileMode tileModeY,
                                             FilterQuality quality,
                                             Scalar* matrix33) {
  if (matrix33) {
    Matrix localMatrix = createMatrix(matrix33);
    return image
        ->makeShader(tileModeX, tileModeY, samplingOptionsForQuality(quality),
                     &localMatrix)
        .release();
  } else {
    return image
        ->makeShader(tileModeX, tileModeY, samplingOptionsForQuality(quality))
        .release();
  }
}
