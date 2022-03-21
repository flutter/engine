// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_PATH_EFFECT_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_PATH_EFFECT_H_

#include <cstddef>
#include <memory>
#include <optional>
#include "flutter/display_list/display_list_attributes.h"
#include "flutter/display_list/types.h"
#include "flutter/fml/logging.h"
#include "include/core/SkRefCnt.h"

namespace flutter {

class DlComposePathEffect;
class DlSumPathEffect;

enum class DlPathEffectType { kSumPathEffect, kComposePathEffect };

class DlPathEffect
    : public DlAttribute<DlPathEffect, SkPathEffect, DlPathEffectType> {
 public:
  static std::shared_ptr<DlPathEffect> From(SkPathEffect* sk_path_effect);

  static std::shared_ptr<DlPathEffect> From(
      sk_sp<SkPathEffect> sk_path_effect) {
    return From(sk_path_effect.get());
  }

  static std::shared_ptr<DlPathEffect> MakeSum(
      std::shared_ptr<DlPathEffect> first,
      std::shared_ptr<DlPathEffect> second);

  static std::shared_ptr<DlPathEffect> MakeCompose(
      std::shared_ptr<DlPathEffect> outer,
      std::shared_ptr<DlPathEffect> inner);

  virtual DlSumPathEffect* asSum() const { return nullptr; }

  virtual DlComposePathEffect* asCompose() const { return nullptr; }
};

class DlPairPathEffect : public DlPathEffect {
 protected:
  DlPairPathEffect(sk_sp<SkPathEffect> pe0, sk_sp<SkPathEffect> pe1)
      : fPE0(std::move(pe0)), fPE1(std::move(pe1)) {}

  sk_sp<SkPathEffect> fPE0;
  sk_sp<SkPathEffect> fPE1;
};

class DlComposePathEffect final : public DlPairPathEffect {
 public:
  DlComposePathEffect(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second)
      : DlPairPathEffect(first, second) {}

  DlComposePathEffect(const DlComposePathEffect& path_effect)
      : DlComposePathEffect(path_effect.fPE0, path_effect.fPE1) {}
  DlComposePathEffect(const DlComposePathEffect* path_effect)
      : DlComposePathEffect(path_effect->fPE0, path_effect->fPE1) {}

  static std::shared_ptr<DlPathEffect> Make(sk_sp<SkPathEffect> outer,
                                            sk_sp<SkPathEffect> inner) {
    if (!outer) {
      return From(inner);
    }
    if (!inner) {
      return From(outer);
    }
    return std::make_shared<DlComposePathEffect>(outer, inner);
  }

  DlPathEffectType type() const override {
    return DlPathEffectType::kComposePathEffect;
  }
  size_t size() const override { return sizeof(*this); }

  std::shared_ptr<DlPathEffect> shared() const override {
    return std::make_shared<DlComposePathEffect>(this);
  }

  const DlComposePathEffect* asCompose() const override { return this; }

  sk_sp<SkPathEffect> skia_object() const override {
    return SkPathEffect::MakeCompose(fPE0, fPE1);
  }

 protected:
  bool equals_(DlPathEffect const& other) const override {
    FML_DCHECK(other.type() == DlPathEffectType::kComposePathEffect);
    auto that = static_cast<DlComposePathEffect const*>(&other);
    return fPE0 == that->fPE0 && fPE1 == that->fPE1;
  }
};

class DlSumPathEffect final : public DlPairPathEffect {
 public:
  DlSumPathEffect(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second)
      : DlPairPathEffect(first, second) {}

  DlSumPathEffect(const DlSumPathEffect& path_effect)
      : DlSumPathEffect(path_effect.fPE0, path_effect.fPE1) {}
  DlSumPathEffect(const DlSumPathEffect* path_effect)
      : DlSumPathEffect(path_effect->fPE0, path_effect->fPE1) {}

  static std::shared_ptr<DlPathEffect> Make(sk_sp<SkPathEffect> first,
                                            sk_sp<SkPathEffect> second) {
    if (!first) {
      return From(second);
    }
    if (!second) {
      return From(first);
    }
    return std::make_shared<DlSumPathEffect>(first, second);
  }

  DlPathEffectType type() const override {
    return DlPathEffectType::kSumPathEffect;
  }
  size_t size() const override { return sizeof(*this); }

  std::shared_ptr<DlPathEffect> shared() const override {
    return std::make_shared<DlSumPathEffect>(this);
  }

  sk_sp<SkPathEffect> skia_object() const override {
    return SkPathEffect::MakeSum(fPE0, fPE1);
  }

  const DlSumPathEffect* asSum() const override { return this; }

 protected:
  bool equals_(DlPathEffect const& other) const override {
    FML_DCHECK(other.type() == DlPathEffectType::kSumPathEffect);
    auto that = static_cast<DlSumPathEffect const*>(&other);
    return fPE0 == that->fPE0 && fPE1 == that->fPE1;
  }
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_PATH_EFFECT_H_
