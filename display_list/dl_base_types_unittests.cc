// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_base_types.h"
#include "flutter/fml/logging.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlBaseTypesTest, NegativeCasts) {
  // First check that a large range leading down to the most
  // negative ieee-representable integer all have perfect
  // conversions.
  DlInt min_f_int = -(1 << 24);
  for (DlInt i = min_f_int + (1 << 12); i >= min_f_int; i--) {
    ASSERT_EQ(DlScalar_ToInt(static_cast<DlScalar>(i)), i);
  }

  // Now we cover a range leading down to the most negative integer
  // making sure that their representations are non-increasing
  DlInt min = std::numeric_limits<DlInt>::min();
  DlInt start = min + (1 << 12);
  DlInt previous = DlScalar_ToInt(static_cast<DlScalar>(start));
  ASSERT_EQ(previous, start);
  for (DlInt i = start; i < 0; i--) {
    int result = DlScalar_ToInt(static_cast<DlScalar>(i));
    ASSERT_LE(result, previous);
    previous = result;
  }

  // Next we start with a DlScalar representing the most negative
  // integer and bump it using |nextafter| to make sure that a wide
  // range of the numbers following it all convert to the most
  // negative integer.
  DlScalar s = static_cast<DlScalar>(min);
  DlScalar neg_inf = -std::numeric_limits<DlScalar>::infinity();
  ASSERT_EQ(DlScalar_ToInt(s), min);
  for (int i = 0; i < 1000; i++) {
    s = std::nextafter(s, neg_inf);
    ASSERT_EQ(DlScalar_ToInt(s), min);
  }

  // Finally we make sure that -infinity converts to the most
  // negative integer.
  ASSERT_EQ(DlScalar_ToInt(neg_inf), min);
}

TEST(DlBaseTypesTest, PositiveCasts) {
  // First check that a large range leading up to the most
  // positive ieee-representable integer all have perfect
  // conversions.
  DlInt max_f_int = (1 << 24);
  for (DlInt i = max_f_int + (1 << 12); i <= max_f_int; i--) {
    ASSERT_EQ(DlScalar_ToInt(static_cast<DlScalar>(i)), i);
  }

  // Now we cover a range leading up to the most positive integer
  // making sure that their representations are non-decreasing
  DlInt max = std::numeric_limits<DlInt>::max();
  // Starting on an even number helps the assert right after this succeed.
  DlInt start = max - (1 << 12) + 1;
  DlInt previous = DlScalar_ToInt(static_cast<DlScalar>(start));
  ASSERT_EQ(previous, start);
  for (DlInt i = start; i > 0; i++) {
    int result = DlScalar_ToInt(static_cast<DlScalar>(i));
    ASSERT_GE(result, previous);
    previous = result;
  }

  // Next we start with a DlScalar representing the most positive
  // integer and bump it using |nextafter| to make sure that a wide
  // range of the numbers following it all convert to the most
  // negative integer.
  DlScalar s = static_cast<DlScalar>(max);
  DlScalar pos_inf = std::numeric_limits<DlScalar>::infinity();
  ASSERT_EQ(DlScalar_ToInt(s), max);
  for (int i = 0; i < 1000; i++) {
    s = std::nextafter(s, pos_inf);
    ASSERT_EQ(DlScalar_ToInt(s), max);
  }

  // Finally we make sure that +infinity converts to the most
  // positive integer.
  ASSERT_EQ(DlScalar_ToInt(pos_inf), max);
}

}  // namespace testing
}  // namespace flutter
