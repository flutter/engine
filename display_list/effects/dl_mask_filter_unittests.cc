// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_mask_filter.h"
#include "flutter/display_list/testing/dl_test_equality.h"
#include "flutter/display_list/utils/dl_comparable.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DisplayListMaskFilter, BlurConstructor) {
  auto filter = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 5.0);
}

TEST(DisplayListMaskFilter, BlurAsBlur) {
  auto filter = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 5.0);
  ASSERT_NE(filter->asBlur(), nullptr);
  ASSERT_EQ(filter->asBlur(), filter.get());
}

TEST(DisplayListMaskFilter, BlurContents) {
  auto filter = DlBlurMaskFilter::Make(DlBlurStyle::kNormal, 5.0);
  ASSERT_EQ(filter->style(), DlBlurStyle::kNormal);
  ASSERT_EQ(filter->sigma(), 5.0);
}

TEST(DisplayListMaskFilter, BlurEquals) {
  auto filter1 = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 5.0);
  auto filter2 = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 5.0);
  TestEquals(*filter1, *filter2);
}

TEST(DisplayListMaskFilter, BlurNotEquals) {
  auto filter1 = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 5.0);
  auto filter2 = DlMaskFilter::MakeBlur(DlBlurStyle::kInner, 5.0);
  auto filter3 = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 6.0);
  TestNotEquals(filter1, filter2, "Blur style differs");
  TestNotEquals(filter1, filter3, "blur radius differs");
}

void testEquals(DlMaskFilter* a, DlMaskFilter* b) {
  // a and b have the same nullness or values
  ASSERT_TRUE(Equals(a, b));
  ASSERT_FALSE(NotEquals(a, b));
  ASSERT_TRUE(Equals(b, a));
  ASSERT_FALSE(NotEquals(b, a));
}

void testNotEquals(DlMaskFilter* a, DlMaskFilter* b) {
  // a and b do not have the same nullness or values
  ASSERT_FALSE(Equals(a, b));
  ASSERT_TRUE(NotEquals(a, b));
  ASSERT_FALSE(Equals(b, a));
  ASSERT_TRUE(NotEquals(b, a));
}

void testEquals(const dl_shared<const DlMaskFilter>& a, DlMaskFilter* b) {
  // a and b have the same nullness or values
  ASSERT_TRUE(Equals(a, b));
  ASSERT_FALSE(NotEquals(a, b));
  ASSERT_TRUE(Equals(b, a));
  ASSERT_FALSE(NotEquals(b, a));
}

void testNotEquals(const dl_shared<const DlMaskFilter>& a, DlMaskFilter* b) {
  // a and b do not have the same nullness or values
  ASSERT_FALSE(Equals(a, b));
  ASSERT_TRUE(NotEquals(a, b));
  ASSERT_FALSE(Equals(b, a));
  ASSERT_TRUE(NotEquals(b, a));
}

void testEquals(const dl_shared<const DlMaskFilter>& a,
                const dl_shared<const DlMaskFilter>& b) {
  // a and b have the same nullness or values
  ASSERT_TRUE(Equals(a, b));
  ASSERT_FALSE(NotEquals(a, b));
  ASSERT_TRUE(Equals(b, a));
  ASSERT_FALSE(NotEquals(b, a));
}

void testNotEquals(const dl_shared<const DlMaskFilter>& a,
                   const dl_shared<const DlMaskFilter>& b) {
  // a and b do not have the same nullness or values
  ASSERT_FALSE(Equals(a, b));
  ASSERT_TRUE(NotEquals(a, b));
  ASSERT_FALSE(Equals(b, a));
  ASSERT_TRUE(NotEquals(b, a));
}

TEST(DisplayListMaskFilter, ComparableTemplates) {
  auto filter1a = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 3.0);
  auto filter1b = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 3.0);
  auto filter2 = DlMaskFilter::MakeBlur(DlBlurStyle::kNormal, 5.0);
  dl_shared<DlMaskFilter> shared_null;

  // null to null
  testEquals(nullptr, nullptr);
  testEquals(shared_null, nullptr);
  testEquals(shared_null, shared_null);

  // ptr to null
  testNotEquals(filter1a.get(), nullptr);
  testNotEquals(filter1b.get(), nullptr);
  testNotEquals(filter2.get(), nullptr);

  // dl_shared to null and shared_null to ptr
  testNotEquals(filter1a, nullptr);
  testNotEquals(filter1b, nullptr);
  testNotEquals(filter2, nullptr);
  testNotEquals(shared_null, filter1a.get());
  testNotEquals(shared_null, filter1b.get());
  testNotEquals(shared_null, filter2.get());

  // ptr to ptr
  testEquals(filter1a.get(), filter1a.get());
  testEquals(filter1a.get(), filter1b.get());
  testEquals(filter1b.get(), filter1b.get());
  testEquals(filter2.get(), filter2.get());
  testNotEquals(filter1a.get(), filter2.get());

  // dl_shared to ptr
  testEquals(filter1a, filter1a.get());
  testEquals(filter1a, filter1b.get());
  testEquals(filter1b, filter1b.get());
  testEquals(filter2, filter2.get());
  testNotEquals(filter1a, filter2.get());
  testNotEquals(filter1b, filter2.get());
  testNotEquals(filter2, filter1a.get());
  testNotEquals(filter2, filter1b.get());

  // dl_shared to dl_shared
  testEquals(filter1a, filter1a);
  testEquals(filter1a, filter1b);
  testEquals(filter1b, filter1b);
  testEquals(filter2, filter2);
  testNotEquals(filter1a, filter2);
  testNotEquals(filter1b, filter2);
  testNotEquals(filter2, filter1a);
  testNotEquals(filter2, filter1b);
}

}  // namespace testing
}  // namespace flutter
