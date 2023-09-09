// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_rstransform.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlRSTransformTest, EmptyConstructor) {
  DlRSTransform rst;
  ASSERT_EQ(rst.scaled_cos(), 1.0f);
  ASSERT_EQ(rst.scaled_sin(), 0.0f);
  ASSERT_EQ(rst.translate_x(), 0.0f);
  ASSERT_EQ(rst.translate_y(), 0.0f);
  ASSERT_TRUE(rst.IsFinite());
  ASSERT_TRUE(rst.IsIdentity());
  ASSERT_EQ(rst.ExtractAngle(), DlAngle::Degrees());
  ASSERT_EQ(rst.ExtractScale(), 1.0f);

  {
    DlFPoint quad[4];
    rst.ToQuad(DlFSize(10, 20), quad);
    ASSERT_EQ(quad[0], DlFPoint(0.0f, 0.0f));
    ASSERT_EQ(quad[1], DlFPoint(10.0f, 0.0f));
    ASSERT_EQ(quad[2], DlFPoint(10.0f, 20.0f));
    ASSERT_EQ(quad[3], DlFPoint(0.0f, 20.0f));
  }

  {
    DlFPoint quad[4];
    rst.ToQuad(10, 20, quad);
    ASSERT_EQ(quad[0], DlFPoint(0.0f, 0.0f));
    ASSERT_EQ(quad[1], DlFPoint(10.0f, 0.0f));
    ASSERT_EQ(quad[2], DlFPoint(10.0f, 20.0f));
    ASSERT_EQ(quad[3], DlFPoint(0.0f, 20.0f));
  }
}

TEST(DlRSTransformTest, DefaultConstructor) {
  DlRSTransform rst = DlRSTransform();
  ASSERT_EQ(rst.scaled_cos(), 1.0f);
  ASSERT_EQ(rst.scaled_sin(), 0.0f);
  ASSERT_EQ(rst.translate_x(), 0.0f);
  ASSERT_EQ(rst.translate_y(), 0.0f);
  ASSERT_TRUE(rst.IsFinite());
  ASSERT_TRUE(rst.IsIdentity());
  ASSERT_EQ(rst.ExtractAngle(), DlAngle::Degrees());
  ASSERT_EQ(rst.ExtractScale(), 1.0f);

  {
    DlFPoint quad[4];
    rst.ToQuad(DlFSize(10, 20), quad);
    ASSERT_EQ(quad[0], DlFPoint(0.0f, 0.0f));
    ASSERT_EQ(quad[1], DlFPoint(10.0f, 0.0f));
    ASSERT_EQ(quad[2], DlFPoint(10.0f, 20.0f));
    ASSERT_EQ(quad[3], DlFPoint(0.0f, 20.0f));
  }

  {
    DlFPoint quad[4];
    rst.ToQuad(10, 20, quad);
    ASSERT_EQ(quad[0], DlFPoint(0.0f, 0.0f));
    ASSERT_EQ(quad[1], DlFPoint(10.0f, 0.0f));
    ASSERT_EQ(quad[2], DlFPoint(10.0f, 20.0f));
    ASSERT_EQ(quad[3], DlFPoint(0.0f, 20.0f));
  }
}

TEST(DlRSTransformTest, ScaledCosSinFactory) {
  // sin and cos of 30 degrees
  DlScalar sin_30 = 0.5f;
  DlScalar cos_30 = sqrt(1 - sin_30 * sin_30);
  DlScalar scale = 2.5f;
  DlRSTransform rst = DlRSTransform::MakeScaledCosSinXY(
      cos_30 * scale, sin_30 * scale, 10.0f, 20.0f);
  ASSERT_EQ(rst.scaled_cos(), cos_30 * scale);
  ASSERT_EQ(rst.scaled_sin(), sin_30 * scale);
  ASSERT_EQ(rst.translate_x(), 10.0f);
  ASSERT_EQ(rst.translate_y(), 20.0f);
  ASSERT_TRUE(rst.IsFinite());
  ASSERT_FALSE(rst.IsIdentity());
  ASSERT_EQ(rst.ExtractAngle(), DlAngle::Degrees(30));
  ASSERT_EQ(rst.ExtractScale(), 2.5f);
}

TEST(DlRSTransformTest, ScaleAngleFactory) {
  DlAngle angle = DlAngle::Degrees(30);
  // sin and cos of 30 degrees
  DlScalar sin_30 = 0.5f;
  DlScalar cos_30 = sqrt(1 - sin_30 * sin_30);
  DlScalar scale = 2.5f;
  DlRSTransform rst =
      DlRSTransform::MakeScaleAngleXY(scale, angle, 10.0f, 20.0f);
  ASSERT_EQ(rst.scaled_cos(), cos_30 * scale);
  ASSERT_EQ(rst.scaled_sin(), sin_30 * scale);
  ASSERT_EQ(rst.translate_x(), 10.0f);
  ASSERT_EQ(rst.translate_y(), 20.0f);
  ASSERT_TRUE(rst.IsFinite());
  ASSERT_FALSE(rst.IsIdentity());
  ASSERT_EQ(rst.ExtractAngle(), DlAngle::Degrees(30));
  ASSERT_EQ(rst.ExtractScale(), 2.5f);
}

TEST(DlRSTransformTest, Assignment) {
  // sin and cos of 30 degrees
  DlScalar sin_30 = 0.5f;
  DlScalar cos_30 = sqrt(1 - sin_30 * sin_30);
  DlScalar scale = 2.5f;
  DlRSTransform original = DlRSTransform::MakeScaledCosSinXY(
      cos_30 * scale, sin_30 * scale, 10.0f, 20.0f);

  DlRSTransform copy = original;
  ASSERT_EQ(copy.scaled_cos(), cos_30 * scale);
  ASSERT_EQ(copy.scaled_sin(), sin_30 * scale);
  ASSERT_EQ(copy.translate_x(), 10.0f);
  ASSERT_EQ(copy.translate_y(), 20.0f);
  ASSERT_TRUE(copy.IsFinite());
  ASSERT_FALSE(copy.IsIdentity());
  ASSERT_EQ(copy.ExtractAngle(), DlAngle::Degrees(30));
  ASSERT_EQ(copy.ExtractScale(), 2.5f);
}

TEST(DlRSTransformTest, ExtractAngleTest) {
  // (-179 => 180] is the open-interval output range of the atan2 function
  // used to compute the angle from the signed sin/cos values.
  for (int d2 = -(180 + 179); d2 <= 180 + 180; d2++) {
    DlScalar d = d2 * 0.5f;
    DlRSTransform rst =
        DlRSTransform::MakeScaleAngleXY(2.5f, DlAngle::Degrees(d), 0.0f, 0.0f);
    ASSERT_TRUE(DlScalar_IsNearlyZero(rst.ExtractAngle().degrees() - d)) << rst;
  }
}

TEST(DlRSTransformTest, ExtractScaleTest) {
  for (int s2 = 1; s2 <= 200; s2++) {
    DlScalar s = s2 * 0.5f;
    DlRSTransform rst =
        DlRSTransform::MakeScaleAngleXY(s, DlAngle::Degrees(30), 0.0f, 0.0f);
    ASSERT_TRUE(DlScalar_IsNearlyZero(rst.ExtractScale() - s)) << rst;
  }
}

TEST(DlRSTransformTest, NaNInfinityProperties) {
  DlScalar nan = std::numeric_limits<DlScalar>::quiet_NaN();
  DlScalar pos_inf = std::numeric_limits<DlScalar>::infinity();
  DlScalar neg_inf = -std::numeric_limits<DlScalar>::infinity();

  // sin and cos of 30 degrees
  DlScalar sin_30 = 0.5f;
  DlScalar cos_30 = sqrt(1 - sin_30 * sin_30);
  DlScalar scale = 2.5f;
  DlRSTransform rst = DlRSTransform::MakeScaledCosSinXY(
      cos_30 * scale, sin_30 * scale, 10.0f, 20.0f);
  ASSERT_EQ(rst.scaled_cos(), cos_30 * scale);
  ASSERT_EQ(rst.scaled_sin(), sin_30 * scale);
  ASSERT_EQ(rst.translate_x(), 10.0f);
  ASSERT_EQ(rst.translate_y(), 20.0f);
  ASSERT_TRUE(rst.IsFinite());
  ASSERT_FALSE(rst.IsIdentity());
  ASSERT_EQ(rst.ExtractAngle(), DlAngle::Degrees(30));
  ASSERT_EQ(rst.ExtractScale(), 2.5f);

  auto test_non_finite = [](const DlRSTransform& rst, int mask,
                            DlScalar non_finite) {
    ASSERT_TRUE(rst.IsFinite());
    std::string label = "with " + std::to_string(non_finite) +  //
                        " in " + std::to_string(mask);

    DlScalar scos = (mask & (1 << 0)) ? non_finite : rst.scaled_cos();
    DlScalar ssin = (mask & (1 << 1)) ? non_finite : rst.scaled_sin();
    DlScalar tx = (mask & (1 << 2)) ? non_finite : rst.translate_x();
    DlScalar ty = (mask & (1 << 3)) ? non_finite : rst.translate_y();

    DlRSTransform rst_nonf =
        DlRSTransform::MakeScaledCosSinXY(scos, ssin, tx, ty);
    if (mask == 0) {
      ASSERT_TRUE(rst_nonf.IsFinite()) << label;
      ASSERT_EQ(rst_nonf, rst) << label;
      ASSERT_EQ(rst.scaled_cos(), rst.scaled_cos()) << label;
      ASSERT_EQ(rst.scaled_sin(), rst.scaled_sin()) << label;
      ASSERT_EQ(rst.translate_x(), rst.translate_x()) << label;
      ASSERT_EQ(rst.translate_y(), rst.translate_y()) << label;
    } else {
      if ((mask & 0x3) == 0) {
        ASSERT_EQ(rst_nonf.scaled_cos(), rst.scaled_cos()) << label;
        ASSERT_EQ(rst_nonf.scaled_sin(), rst.scaled_sin()) << label;
        ASSERT_EQ(rst_nonf.ExtractAngle(), rst.ExtractAngle()) << label;
        ASSERT_EQ(rst_nonf.ExtractScale(), rst.ExtractScale()) << label;
      } else {
        ASSERT_EQ(rst_nonf.scaled_cos(), 1.0f) << label;
        ASSERT_EQ(rst_nonf.scaled_sin(), 0.0f) << label;
        ASSERT_EQ(rst_nonf.ExtractAngle(), DlAngle()) << label;
        ASSERT_EQ(rst_nonf.ExtractScale(), 1.0f) << label;
      }
      if ((mask & 0xc) == 0) {
        ASSERT_TRUE(rst_nonf.IsFinite()) << label;
        ASSERT_EQ(rst_nonf.translate_x(), rst.translate_x()) << label;
        ASSERT_EQ(rst_nonf.translate_y(), rst.translate_y()) << label;
      } else if (std::isnan(non_finite)) {
        ASSERT_TRUE(rst_nonf.IsFinite()) << label;
        if ((mask & 0x4) == 0) {
          ASSERT_EQ(rst_nonf.translate_x(), 10.0f) << label;
        } else {
          ASSERT_EQ(rst_nonf.translate_x(), 0.0f) << label;
        }
        if ((mask & 0x8) == 0) {
          ASSERT_EQ(rst_nonf.translate_y(), 20.0f) << label;
        } else {
          ASSERT_EQ(rst_nonf.translate_y(), 0.0f) << label;
        }
      } else {
        ASSERT_FALSE(rst_nonf.IsFinite()) << label;
        ASSERT_EQ(rst_nonf.translate_x(), tx) << label;
        ASSERT_EQ(rst_nonf.translate_y(), ty) << label;
      }
    }
  };

  for (int mask = 0; mask < 16; mask++) {
    test_non_finite(rst, mask, nan);
    test_non_finite(rst, mask, pos_inf);
    test_non_finite(rst, mask, neg_inf);
  }
}

}  // namespace testing
}  // namespace flutter
