// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_attributes_testing.h"
#include "flutter/display_list/display_list_builder.h"
#include "flutter/display_list/display_list_comparable.h"
#include "flutter/display_list/types.h"
#include "gtest/gtest.h"
#include "include/core/SkPath.h"

namespace flutter {
namespace testing {

const SkScalar TestDashes1[] = {4.0, 2.0};
const SkScalar TestDashes2[] = {1.0, 1.5};

TEST(DisplayListPathEffect, BuilderSetGet) {
  auto dash_path_effect = DlDashPathEffect::Make(TestDashes1, 2, 0.0);
  DisplayListBuilder builder;
  ASSERT_EQ(builder.getPathEffect(), nullptr);
  builder.setPathEffect(dash_path_effect.get());
  ASSERT_NE(builder.getPathEffect(), nullptr);
  ASSERT_TRUE(Equals(builder.getPathEffect(),
                     static_cast<DlPathEffect*>(dash_path_effect.get())));
  builder.setPathEffect(nullptr);
  ASSERT_EQ(builder.getPathEffect(), nullptr);
}

TEST(DisplayListPathEffect, FromSkiaNullPathEffect) {
  std::shared_ptr<DlPathEffect> path_effect = DlPathEffect::From(nullptr);
  ASSERT_EQ(path_effect, nullptr);
  ASSERT_EQ(path_effect.get(), nullptr);
}

TEST(DisplayListPathEffect, FromSkiaPathEffect) {
  sk_sp<SkPathEffect> sk_path_effect =
      SkDashPathEffect::Make(TestDashes2, 2, 0.0);
  std::shared_ptr<DlPathEffect> dl_path_effect =
      DlPathEffect::From(sk_path_effect);

  ASSERT_EQ(dl_path_effect->type(), DlPathEffectType::kDash);
  // We cannot recapture the dash parameters from an SkDashPathEffect
  ASSERT_EQ(dl_path_effect->asDash(), dl_path_effect.get());
  ASSERT_TRUE(
      Equals(dl_path_effect, DlDashPathEffect::Make(TestDashes2, 2, 0.0)));
}

TEST(DisplayListPathEffect, EffectShared) {
  auto effect = DlDashPathEffect::Make(TestDashes2, 2, 0.0);
  ASSERT_TRUE(Equals(effect->shared(), effect));
}

TEST(DisplayListPathEffect, DashEffectAsDash) {
  auto effect = DlDashPathEffect::Make(TestDashes2, 2, 0.0);
  ASSERT_NE(effect->asDash(), nullptr);
  ASSERT_EQ(effect->asDash(), effect.get());
}

TEST(DisplayListPathEffect, DashEffectEquals) {
  auto effect1 = DlDashPathEffect::Make(TestDashes2, 2, 0.0);
  auto effect2 = DlDashPathEffect::Make(TestDashes2, 2, 0.0);
  ASSERT_TRUE(Equals(effect1, effect2));
  ASSERT_TRUE(Equals(effect1->shared(), effect2->shared()));
}

TEST(DisplayListPathEffect, BlurNotEquals) {
  auto effect1 = DlDashPathEffect::Make(TestDashes1, 2, 0.0);
  auto effect2 = DlDashPathEffect::Make(TestDashes2, 2, 0.0);
  auto effect3 = DlDashPathEffect::Make(TestDashes2, 3, 0.0);
  ASSERT_NE(effect1, effect2);
  ASSERT_NE(effect2, effect3);
  ASSERT_NE(effect1->shared(), effect2->shared());
  ASSERT_NE(effect2->shared(), effect3->shared());
}

TEST(DisplayListPathEffect, UnknownConstructor) {
  DlUnknownPathEffect path_effect(SkDashPathEffect::Make(TestDashes1, 2, 0.0));
}

TEST(DisplayListPathEffect, UnknownShared) {
  DlUnknownPathEffect path_effect(SkDashPathEffect::Make(TestDashes1, 2, 0.0));
  ASSERT_NE(path_effect.shared().get(), &path_effect);
  ASSERT_EQ(*path_effect.shared(), path_effect);
}

TEST(DisplayListPathEffect, UnknownContents) {
  sk_sp<SkPathEffect> sk_effect = SkDashPathEffect::Make(TestDashes1, 2, 0.0);
  DlUnknownPathEffect effect(sk_effect);
  ASSERT_EQ(effect.skia_object(), sk_effect);
  ASSERT_EQ(effect.skia_object().get(), sk_effect.get());
}

TEST(DisplayListPathEffect, UnknownEquals) {
  sk_sp<SkPathEffect> sk_effect = SkDashPathEffect::Make(TestDashes1, 2, 0.0);
  DlUnknownPathEffect effect1(sk_effect);
  DlUnknownPathEffect effect2(sk_effect);
  TestEquals(effect1, effect1);
}

TEST(DisplayListPathEffect, UnknownNotEquals) {
  // Even though the effect is the same, it is a different instance
  // and we cannot currently tell them apart because the Skia
  // DashEffect::Make objects do not implement ==
  DlUnknownPathEffect path_effect1(SkDashPathEffect::Make(TestDashes1, 2, 0.0));
  DlUnknownPathEffect path_effect2(SkDashPathEffect::Make(TestDashes1, 2, 0.0));
  TestNotEquals(path_effect1, path_effect2,
                "SkDashPathEffect instance differs");
}

void testEquals(DlPathEffect* a, DlPathEffect* b) {
  // a and b have the same nullness or values
  ASSERT_TRUE(Equals(a, b));
  ASSERT_FALSE(NotEquals(a, b));
  ASSERT_TRUE(Equals(b, a));
  ASSERT_FALSE(NotEquals(b, a));
}

void testNotEquals(DlPathEffect* a, DlPathEffect* b) {
  // a and b do not have the same nullness or values
  ASSERT_FALSE(Equals(a, b));
  ASSERT_TRUE(NotEquals(a, b));
  ASSERT_FALSE(Equals(b, a));
  ASSERT_TRUE(NotEquals(b, a));
}

void testEquals(std::shared_ptr<const DlPathEffect> a, DlPathEffect* b) {
  // a and b have the same nullness or values
  ASSERT_TRUE(Equals(a, b));
  ASSERT_FALSE(NotEquals(a, b));
  ASSERT_TRUE(Equals(b, a));
  ASSERT_FALSE(NotEquals(b, a));
}

void testNotEquals(std::shared_ptr<const DlPathEffect> a, DlPathEffect* b) {
  // a and b do not have the same nullness or values
  ASSERT_FALSE(Equals(a, b));
  ASSERT_TRUE(NotEquals(a, b));
  ASSERT_FALSE(Equals(b, a));
  ASSERT_TRUE(NotEquals(b, a));
}

void testEquals(std::shared_ptr<const DlPathEffect> a,
                std::shared_ptr<const DlPathEffect> b) {
  // a and b have the same nullness or values
  ASSERT_TRUE(Equals(a, b));
  ASSERT_FALSE(NotEquals(a, b));
  ASSERT_TRUE(Equals(b, a));
  ASSERT_FALSE(NotEquals(b, a));
}

void testNotEquals(std::shared_ptr<const DlPathEffect> a,
                   std::shared_ptr<const DlPathEffect> b) {
  // a and b do not have the same nullness or values
  ASSERT_FALSE(Equals(a, b));
  ASSERT_TRUE(NotEquals(a, b));
  ASSERT_FALSE(Equals(b, a));
  ASSERT_TRUE(NotEquals(b, a));
}

TEST(DisplayListPathEffect, ComparableTemplates) {
  auto effect1 = DlDashPathEffect::Make(TestDashes1, 2, 0.0);
  auto effect2 = DlDashPathEffect::Make(TestDashes1, 2, 0.0);
  auto effect3 = DlDashPathEffect::Make(TestDashes2, 3, 0.0);
  std::shared_ptr<DlPathEffect> shared_null;

  // null to null
  testEquals(nullptr, nullptr);
  testEquals(shared_null, nullptr);
  testEquals(shared_null, shared_null);

  // ptr to null
  testNotEquals(effect1.get(), nullptr);
  testNotEquals(effect2.get(), nullptr);
  testNotEquals(effect3.get(), nullptr);

  // shared_ptr to null and shared_null to ptr
  testNotEquals(effect1->shared(), nullptr);
  testNotEquals(effect2->shared(), nullptr);
  testNotEquals(effect3->shared(), nullptr);
  testNotEquals(shared_null, effect1.get());
  testNotEquals(shared_null, effect2.get());
  testNotEquals(shared_null, effect3.get());

  // ptr to ptr
  testEquals(effect1, effect1);
  testEquals(effect1, effect2);
  testEquals(effect3, effect3);
  testEquals(effect2, effect2);

  // shared_ptr to ptr
  testEquals(effect1->shared(), effect1);
  testEquals(effect1->shared(), effect2);
  testEquals(effect2->shared(), effect2);
  testEquals(effect3->shared(), effect3);
  testNotEquals(effect1->shared(), effect3);
  testNotEquals(effect2->shared(), effect3);
  testNotEquals(effect3->shared(), effect1);
  testNotEquals(effect3->shared(), effect2);

  // shared_ptr to shared_ptr
  testEquals(effect1->shared(), effect1->shared());
  testEquals(effect1->shared(), effect2->shared());
  testEquals(effect2->shared(), effect2->shared());
  testEquals(effect3->shared(), effect3->shared());
  testNotEquals(effect1->shared(), effect3->shared());
  testNotEquals(effect2->shared(), effect3->shared());
  testNotEquals(effect3->shared(), effect1->shared());
  testNotEquals(effect3->shared(), effect2->shared());
}

}  // namespace testing
}  // namespace flutter
