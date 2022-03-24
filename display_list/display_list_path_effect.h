// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_PATH_EFFECT_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_PATH_EFFECT_H_

#include "flutter/display_list/display_list_attributes.h"
#include "flutter/display_list/types.h"
#include "flutter/fml/logging.h"
#include "include/core/SkScalar.h"

namespace flutter {

class DlDashPathEffect;

// The DisplayList PathEffect class. This class implements all of the
// facilities and adheres to the design goals of the |DlAttribute| base
// class.

// An enumerated type for the recognized PathEffect operations.
// In current Flutter we only use the DashPathEffect.
// And another PathEffect outside of the recognized types is needed
// then a |kUnknown| type that simply defers to an SkPathEffect is
// provided as a fallback.
enum class DlPathEffectType {
  kDash,
  kUnknown,
};

class DlPathEffect
    : public DlAttribute<DlPathEffect, SkPathEffect, DlPathEffectType> {
 public:
  static std::shared_ptr<DlPathEffect> From(SkPathEffect* sk_path_effect);

  static std::shared_ptr<DlPathEffect> From(
      sk_sp<SkPathEffect> sk_path_effect) {
    return From(sk_path_effect.get());
  }

  virtual const DlDashPathEffect* asDash() const { return nullptr; }

 protected:
  DlPathEffect() = default;

 private:
  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(DlPathEffect);
};

/// The DashPathEffect which specifies modifying the path effect, and it
/// only affects storked paths.
/// intervals: array containing an even number of entries (>=2), with
/// the even indices specifying the length of "on" intervals, and the odd
/// indices specifying the length of "off" intervals. This array will be
/// copied in Make, and can be disposed of freely after.
/// count: number of elements in the intervals array
/// phase: offset into the intervals array (mod the sum of all of the
/// intervals).
///
/// For example: if intervals[] = {10, 20}, count = 2, and phase = 25,
/// this will set up a dashed path like so:
/// 5 pixels off
/// 10 pixels on
/// 20 pixels off
/// 10 pixels on
/// 20 pixels off
/// ...
/// A phase of -5, 25, 55, 85, etc. would all result in the same path,
/// because the sum of all the intervals is 30.
///
class DlDashPathEffect final : public DlPathEffect {
 public:
  static std::shared_ptr<DlPathEffect> Make(const SkScalar intervals[],
                                            int count,
                                            SkScalar phase);

  DlPathEffectType type() const override { return DlPathEffectType::kDash; }
  size_t size() const override {
    return sizeof(*this) + sizeof(SkScalar) * count_;
  }

  std::shared_ptr<DlPathEffect> shared() const override {
    return Make(intervals_, count_, phase_);
  }

  const DlDashPathEffect* asDash() const override { return this; }

  sk_sp<SkPathEffect> skia_object() const override {
    return SkDashPathEffect::Make(intervals_, count_, phase_);
  }

  SkScalar* intervals() const { return intervals_; }

 protected:
  bool equals_(DlPathEffect const& other) const override {
    FML_DCHECK(other.type() == DlPathEffectType::kDash);
    auto that = static_cast<DlDashPathEffect const*>(&other);
    return count_ == that->count_ && base_equals_(that) &&
           phase_ == that->phase_;
  }

 private:
  bool base_equals_(DlDashPathEffect const* other) const {
    // intervals not nullptr, that has value
    if (intervals_ != nullptr && other != nullptr) {
      for (int i = 0; i < count_; i++) {
        if (intervals_[i] != other->intervals_[i]) {
          return false;
        }
      }
    }
    return true;
  }

  DlDashPathEffect(const SkScalar intervals[], int count, SkScalar phase)
      : count_(count), phase_(phase) {
    intervals_ = reinterpret_cast<SkScalar*>(this + 1);
    if (intervals != nullptr) {
      memcpy(intervals_, intervals, sizeof(SkScalar) * count);
    }
  }

  DlDashPathEffect(const DlDashPathEffect* dash_effect)
      : DlDashPathEffect(dash_effect->intervals_,
                         dash_effect->count_,
                         dash_effect->phase_) {}

  SkScalar* intervals_;
  int count_;
  SkScalar phase_;

  friend class DisplayListBuilder;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(DlDashPathEffect);
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

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_PATH_EFFECT_H_
