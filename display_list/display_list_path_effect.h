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

namespace flutter {

class DlComposePathEffect;
class DlSumPathEffect;
class DlCornerPathEffect;
class DlDashPathEffect;
class DlDiscretePathEffect;

enum class DlPathEffectType {
  kCorner,
  kDash,
  kDiscrete,
  kSumPathEffect,
  kComposePathEffect,
  kUnknown
};

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

  virtual const DlCornerPathEffect* asCorner() const { return nullptr; }

  virtual const DlDashPathEffect* asDash() const { return nullptr; }

  virtual const DlDiscretePathEffect* asDiscrete() const { return nullptr; }

  virtual const DlSumPathEffect* asSum() const { return nullptr; }
  virtual const DlComposePathEffect* asCompose() const { return nullptr; }
};

class DlCornerPathEffect : public DlPathEffect {
 public:
  DlCornerPathEffect(SkScalar radius) : radius_(radius) {}

  DlCornerPathEffect(const DlCornerPathEffect& corner_effect)
      : DlCornerPathEffect(corner_effect.radius_) {}
  DlCornerPathEffect(const DlCornerPathEffect* corner_effect)
      : DlCornerPathEffect(corner_effect->radius_) {}

  static std::shared_ptr<DlCornerPathEffect> Make(SkScalar radius) {
    return std::make_shared<DlCornerPathEffect>(radius);
  }

  DlPathEffectType type() const override { return DlPathEffectType::kCorner; }
  size_t size() const override { return sizeof(*this); }

  std::shared_ptr<DlPathEffect> shared() const override {
    return std::make_shared<DlCornerPathEffect>(this);
  }

  const DlCornerPathEffect* asCorner() const override { return this; }

  sk_sp<SkPathEffect> skia_object() const override {
    return SkCornerPathEffect::Make(radius_);
  }

 protected:
  bool equals_(DlPathEffect const& other) const override {
    FML_DCHECK(other.type() == DlPathEffectType::kCorner);
    auto that = static_cast<DlCornerPathEffect const*>(&other);
    return radius_ == that->radius_;
  }

  SkScalar radius_;
};

class DlDashPathEffect : public DlPathEffect {
 public:
  DlDashPathEffect(const SkScalar intervals[], int count, SkScalar phase)
      : intervals_(intervals), count_(count), phase_(phase) {}

  DlDashPathEffect(const DlDashPathEffect& dash_effect)
      : DlDashPathEffect(dash_effect.intervals_,
                         dash_effect.count_,
                         dash_effect.phase_) {}
  DlDashPathEffect(const DlDashPathEffect* dash_effect)
      : DlDashPathEffect(dash_effect->intervals_,
                         dash_effect->count_,
                         dash_effect->phase_) {}

  static std::shared_ptr<DlDashPathEffect> Make(const SkScalar intervals[],
                                                int count,
                                                SkScalar phase) {
    return std::make_shared<DlDashPathEffect>(intervals, count, phase);
  }

  DlPathEffectType type() const override { return DlPathEffectType::kDash; }
  size_t size() const override { return sizeof(*this); }

  std::shared_ptr<DlPathEffect> shared() const override {
    return std::make_shared<DlDashPathEffect>(this);
  }

  const DlDashPathEffect* asDash() const override { return this; }

  sk_sp<SkPathEffect> skia_object() const override {
    return SkDashPathEffect::Make(intervals_, count_, phase_);
  }

 protected:
  bool equals_(DlPathEffect const& other) const override {
    FML_DCHECK(other.type() == DlPathEffectType::kDash);
    auto that = static_cast<DlDashPathEffect const*>(&other);
    return intervals_ == that->intervals_ && count_ == that->count_ &&
           phase_ == that->phase_;
  }

  const SkScalar* intervals_;
  int count_;
  SkScalar phase_;
};

class DlDiscretePathEffect : public DlPathEffect {
 public:
  DlDiscretePathEffect(SkScalar segLength,
                       SkScalar dev,
                       uint32_t seedAssist = 0)
      : segLength_(segLength), dev_(dev), seedAssist_(seedAssist) {}

  DlDiscretePathEffect(const DlDiscretePathEffect& discrete_effect)
      : DlDiscretePathEffect(discrete_effect.segLength_,
                             discrete_effect.dev_,
                             discrete_effect.seedAssist_) {}
  DlDiscretePathEffect(const DlDiscretePathEffect* discrete_effect)
      : DlDiscretePathEffect(discrete_effect->segLength_,
                             discrete_effect->dev_,
                             discrete_effect->seedAssist_) {}

  DlPathEffectType type() const override { return DlPathEffectType::kDiscrete; }
  size_t size() const override { return sizeof(*this); }

  static std::shared_ptr<DlDiscretePathEffect> Make(SkScalar segLength,
                                                    SkScalar dev,
                                                    uint32_t seedAssist = 0) {
    return std::make_shared<DlDiscretePathEffect>(segLength, dev, seedAssist);
  }

  std::shared_ptr<DlPathEffect> shared() const override {
    return std::make_shared<DlDiscretePathEffect>(this);
  }

  const DlDiscretePathEffect* asDiscrete() const override { return this; }

  sk_sp<SkPathEffect> skia_object() const override {
    return SkDiscretePathEffect::Make(segLength_, dev_, seedAssist_);
  }

 protected:
  bool equals_(DlPathEffect const& other) const override {
    FML_DCHECK(other.type() == DlPathEffectType::kDiscrete);
    auto that = static_cast<DlDiscretePathEffect const*>(&other);
    return segLength_ == that->segLength_ && dev_ == that->dev_ &&
           seedAssist_ == that->seedAssist_;
  }

  SkScalar segLength_;
  SkScalar dev_;
  uint32_t seedAssist_ = 0;
};

class DlUnknownPathEffect final : public DlPathEffect {
 public:
  DlUnknownPathEffect(sk_sp<SkPathEffect> effect)
      : sk_path_effect_(std::move(effect)) {}
  DlUnknownPathEffect(const DlUnknownPathEffect& effect)
      : DlUnknownPathEffect(effect.sk_path_effect_) {}
  DlUnknownPathEffect(const DlUnknownPathEffect* effect)
      : DlUnknownPathEffect(effect->sk_path_effect_) {}

  DlPathEffectType type() const override { return DlPathEffectType::kUnknown; }
  size_t size() const override { return sizeof(*this); }

  std::shared_ptr<DlPathEffect> shared() const override {
    return std::make_shared<DlUnknownPathEffect>(this);
  }

  sk_sp<SkPathEffect> skia_object() const override { return sk_path_effect_; }

  virtual ~DlUnknownPathEffect() = default;

 protected:
  bool equals_(const DlPathEffect& other) const override {
    FML_DCHECK(other.type() == DlPathEffectType::kUnknown);
    auto that = static_cast<DlUnknownPathEffect const*>(&other);
    return sk_path_effect_ == that->sk_path_effect_;
  }

 private:
  sk_sp<SkPathEffect> sk_path_effect_;
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
