// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_runtime_effect.h"

#include "third_party/skia/include/core/SkRefCnt.h"

namespace flutter {

//------------------------------------------------------------------------------
/// DlRuntimeEffect
///

std::shared_ptr<DlRuntimeEffect> DlRuntimeEffect::MakeSkia(
    const sk_sp<SkRuntimeEffect>& runtime_effect) {
  return std::shared_ptr<DlRuntimeEffectSkia>(
      new DlRuntimeEffectSkia(runtime_effect));
}

std::shared_ptr<DlRuntimeEffect> DlRuntimeEffect::MakeImpeller(
    const std::shared_ptr<impeller::RuntimeStage>& runtime_stage) {
  return std::shared_ptr<DlRuntimeEffectImpeller>(
      new DlRuntimeEffectImpeller(runtime_stage));
}

//------------------------------------------------------------------------------
/// DlRuntimeEffectSkia
///

DlRuntimeEffectSkia::DlRuntimeEffectSkia(
    const sk_sp<SkRuntimeEffect>& runtime_effect)
    : skia_runtime_effect_(runtime_effect) {}

sk_sp<SkRuntimeEffect> DlRuntimeEffectSkia::skia_runtime_effect() const {
  return skia_runtime_effect_;
}

std::shared_ptr<impeller::RuntimeStage> DlRuntimeEffectSkia::runtime_stage()
    const {
  return nullptr;
}

//------------------------------------------------------------------------------
/// DlRuntimeEffectImpeller
///

DlRuntimeEffectImpeller::DlRuntimeEffectImpeller(
    const std::shared_ptr<impeller::RuntimeStage>& runtime_stage)
    : runtime_stage_(runtime_stage){};

sk_sp<SkRuntimeEffect> DlRuntimeEffectImpeller::skia_runtime_effect() const {
  return nullptr;
}

std::shared_ptr<impeller::RuntimeStage> DlRuntimeEffectImpeller::runtime_stage()
    const {
  return runtime_stage_;
}

}  // namespace flutter
