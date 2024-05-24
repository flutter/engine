// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"
#include "wrappers.h"

using namespace Skwasm;

SKWASM_EXPORT SkShader* shader_createLinearGradient(
    Point* endPoints,  // Two points
    SkColor* colors,
    Scalar* stops,
    int count,  // Number of stops/colors
    TileMode tileMode,
    Scalar* matrix33  // Can be nullptr
) {
  if (matrix33) {
    SkMatrix localMatrix = createMatrix(matrix33);
    return SkGradientShader::MakeLinear(endPoints, colors, stops, count,
                                        tileMode, 0, &localMatrix)
        .release();
  } else {
    return SkGradientShader::MakeLinear(endPoints, colors, stops, count,
                                        tileMode)
        .release();
  }
}

SKWASM_EXPORT SkShader* shader_createRadialGradient(Scalar centerX,
                                                    Scalar centerY,
                                                    Scalar radius,
                                                    SkColor* colors,
                                                    Scalar* stops,
                                                    int count,
                                                    TileMode tileMode,
                                                    Scalar* matrix33) {
  if (matrix33) {
    SkMatrix localMatrix = createMatrix(matrix33);
    return SkGradientShader::MakeRadial({centerX, centerY}, radius, colors,
                                        stops, count, tileMode, 0, &localMatrix)
        .release();
  } else {
    return SkGradientShader::MakeRadial({centerX, centerY}, radius, colors,
                                        stops, count, tileMode)
        .release();
  }
}

SKWASM_EXPORT SkShader* shader_createConicalGradient(
    Point* endPoints,  // Two points
    Scalar startRadius,
    Scalar endRadius,
    SkColor* colors,
    Scalar* stops,
    int count,
    TileMode tileMode,
    Scalar* matrix33) {
  if (matrix33) {
    SkMatrix localMatrix = createMatrix(matrix33);
    return SkGradientShader::MakeTwoPointConical(
               endPoints[0], startRadius, endPoints[1], endRadius, colors,
               stops, count, tileMode, 0, &localMatrix)
        .release();

  } else {
    return SkGradientShader::MakeTwoPointConical(endPoints[0], startRadius,
                                                 endPoints[1], endRadius,
                                                 colors, stops, count, tileMode)
        .release();
  }
}

SKWASM_EXPORT SkShader* shader_createSweepGradient(Scalar centerX,
                                                   Scalar centerY,
                                                   SkColor* colors,
                                                   Scalar* stops,
                                                   int count,
                                                   TileMode tileMode,
                                                   Scalar startAngle,
                                                   Scalar endAngle,
                                                   Scalar* matrix33) {
  if (matrix33) {
    SkMatrix localMatrix = createMatrix(matrix33);
    return SkGradientShader::MakeSweep(centerX, centerY, colors, stops, count,
                                       tileMode, startAngle, endAngle, 0,
                                       &localMatrix)
        .release();
  } else {
    return SkGradientShader::MakeSweep(centerX, centerY, colors, stops, count,
                                       tileMode, startAngle, endAngle, 0,
                                       nullptr)
        .release();
  }
}

SKWASM_EXPORT void shader_dispose(SkShader* shader) {
  shader->unref();
}

SKWASM_EXPORT SkRuntimeEffect* runtimeEffect_create(SkString* source) {
  auto result = SkRuntimeEffect::MakeForShader(*source);
  if (result.effect == nullptr) {
    printf("Failed to compile shader. Error text:\n%s",
           result.errorText.data());
    return nullptr;
  } else {
    return result.effect.release();
  }
}

SKWASM_EXPORT void runtimeEffect_dispose(SkRuntimeEffect* effect) {
  effect->unref();
}

SKWASM_EXPORT size_t runtimeEffect_getUniformSize(SkRuntimeEffect* effect) {
  return effect->uniformSize();
}

SKWASM_EXPORT SkShader* shader_createRuntimeEffectShader(
    SkRuntimeEffect* runtimeEffect,
    SkData* uniforms,
    SkShader** children,
    size_t childCount) {
  std::vector<sk_sp<SkShader>> childPointers;
  for (size_t i = 0; i < childCount; i++) {
    childPointers.emplace_back(sk_ref_sp<SkShader>(children[i]));
  }
  return runtimeEffect
      ->makeShader(SkData::MakeWithCopy(uniforms->data(), uniforms->size()),
                   childPointers.data(), childCount, nullptr)
      .release();
}

SKWASM_EXPORT SkShader* shader_createFromImage(SkImage* image,
                                               TileMode tileModeX,
                                               TileMode tileModeY,
                                               FilterQuality quality,
                                               Scalar* matrix33) {
  if (matrix33) {
    SkMatrix localMatrix = createMatrix(matrix33);
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
