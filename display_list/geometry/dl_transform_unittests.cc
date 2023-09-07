// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_transform.h"

#include "fml/logging.h"
#include "gtest/gtest.h"

#include "third_party/skia/include/core/SkM44.h"
#include "third_party/skia/include/core/SkMatrix.h"

namespace flutter {
namespace testing {

// ASSERT_TRUE(result.has_value()) should be enough to prevent the test
// code that follows it to run, but clang-tidy doesn't seem to recognize
// that macro expansion as protective. This macro is more proactive
// about ending execution at the place where the statement lives.
#define ENFORCE_TRUE(condition) \
  do {                          \
    if (!(condition)) {         \
      ASSERT_TRUE(condition);   \
      return;                   \
    }                           \
  } while (0)

static std::ostream& operator<<(std::ostream& os, const SkM44& t) {
  // clang-format off
  return os << "SkM44<RowMajor>(["
      << "[" << t.rc(0, 0) << ", " << t.rc(0, 1) << ", " << t.rc(0, 2) << ", " << t.rc(0, 3) << "], "
      << "[" << t.rc(1, 0) << ", " << t.rc(1, 1) << ", " << t.rc(1, 2) << ", " << t.rc(1, 3) << "], "
      << "[" << t.rc(2, 0) << ", " << t.rc(2, 1) << ", " << t.rc(2, 2) << ", " << t.rc(2, 3) << "], "
      << "[" << t.rc(3, 0) << ", " << t.rc(3, 1) << ", " << t.rc(3, 2) << ", " << t.rc(3, 3) << "]"
      << ")";
  // clang-format on
}

static constexpr DlScalar kIdentityMatrix[16] = {
    // clang-format off
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
    // clang-format on
};

static bool CompareRowMajor(DlTransform transform, const DlScalar m[16]) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (transform.rc(r, c) != m[r * 4 + c]) {
        FML_LOG(ERROR) << "(transform.rc(" << r << ", " << c
                       << ") == " << transform.rc(r, c) << ") != (m["
                       << (r * 4 + c) << "] == " << m[r * 4 + c] << ")";
        return false;
      }
    }
  }
  return true;
}

template <typename FT>
std::enable_if_t<std::is_floating_point_v<FT>, FT> ulp(FT x) {
  if (x > 0) {
    return std::nexttoward(x, std::numeric_limits<FT>::infinity()) - x;
  } else {
    return x - std::nexttoward(x, -std::numeric_limits<FT>::infinity());
  }
}

static int diff_ulps(DlScalar a, DlScalar b) {
  DlScalar u = std::max(std::max(ulp(a), ulp(b)),  //
                        ulp(abs(a - b)));
  return ceilf(abs(a - b) / u);
}

static bool CloseEnough(DlScalar result,
                        DlScalar expected,
                        DlScalar max_diff = kDlScalar_NearlyZero,
                        int max_ulps = 1) {
  if (abs(result - expected) > max_diff) {
    if (diff_ulps(result, expected) > max_ulps) {
      return false;
    }
  }
  return true;
}

static bool IsClose(const DlFPoint& result,
                    const DlFPoint& expected,
                    DlScalar max_diff = kDlScalar_NearlyZero,
                    int max_ulps = 3) {
  if (!CloseEnough(result.x(), expected.x(), max_diff, max_ulps) ||
      !CloseEnough(result.y(), expected.y(), max_diff, max_ulps)) {
    FML_LOG(ERROR) << "Result: " << result;
    FML_LOG(ERROR) << "Expected: " << expected;
    FML_LOG(ERROR) << "Differences = "  //
                   << (result.x() - expected.x()) << ", "
                   << (result.y() - expected.y());
    FML_LOG(ERROR) << "Difference ulps = "  //
                   << diff_ulps(result.x(), expected.x()) << ", "
                   << diff_ulps(result.y(), expected.y());
    return false;
  }
  return true;
}

static bool IsClose(const DlFPoint& result,
                    const SkPoint& expected,
                    DlScalar max_diff = kDlScalar_NearlyZero,
                    int max_ulps = 3) {
  return IsClose(result, DlFPoint(expected.x(), expected.y()), max_diff,
                 max_ulps);
}

static bool IsClose(const DlFPoint& result,
                    const SkV4& expected,
                    DlScalar max_diff = kDlScalar_NearlyZero,
                    int max_ulps = 3) {
  return IsClose(result, DlFPoint(expected.x, expected.y));
}

static bool IsClose(const DlTransform& result,
                    const DlTransform& expected,
                    DlScalar max_diff = kDlScalar_NearlyZero,
                    int max_ulps = 1) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      DlScalar rv = result.rc(r, c);
      DlScalar ev = expected.rc(r, c);
      if (!CloseEnough(rv, ev, max_diff, max_ulps)) {
        FML_LOG(ERROR) << "At (" << r << ", " << c << "), " << rv << " !~ "
                       << ev;
        FML_LOG(ERROR) << "Result: " << result;
        FML_LOG(ERROR) << "Expected: " << expected;
        FML_LOG(ERROR) << "Difference: " << (rv - ev);
        FML_LOG(ERROR) << "Difference ulps: " << diff_ulps(rv, ev);
        return false;
      }
    }
  }
  return true;
}

static bool IsClose(const DlTransform& result,
                    const SkM44& expected,
                    DlScalar max_diff = kDlScalar_NearlyZero,
                    int max_ulps = 1) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      DlScalar rv = result.rc(r, c);
      DlScalar ev = expected.rc(r, c);
      if (!CloseEnough(rv, ev, max_diff, max_ulps)) {
        FML_LOG(ERROR) << "At (" << r << ", " << c << "), " << rv << " !~ "
                       << ev;
        FML_LOG(ERROR) << "Result: " << result;
        FML_LOG(ERROR) << "Expected: " << expected;
        FML_LOG(ERROR) << "Difference: " << (rv - ev);
        FML_LOG(ERROR) << "Difference ulps: " << diff_ulps(rv, ev);
        return false;
      }
    }
  }
  return true;
}

static bool IsClose(const DlTransform& result,
                    const SkMatrix& expected,
                    DlScalar max_diff = kDlScalar_NearlyZero,
                    int max_ulps = 1) {
  return IsClose(result, SkM44(expected), max_diff, max_ulps);
}

TEST(DlTransformTest, ImplicitConstructor) {
  DlTransform transform;

  EXPECT_TRUE(transform.IsIdentity());
  EXPECT_TRUE(transform.IsFinite());
  EXPECT_TRUE(transform.Is2D());
  EXPECT_TRUE(transform.IsTranslate());
  EXPECT_TRUE(transform.IsInvertible());
  EXPECT_TRUE(transform.IsScaleTranslate());
  EXPECT_TRUE(transform.RectStaysRect());
  EXPECT_FALSE(transform.HasPerspective());
  EXPECT_EQ(transform.Determinant(), kDlScalar_One);
  EXPECT_TRUE(CompareRowMajor(transform, kIdentityMatrix));
  EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(12.0f, 17.0f));
  EXPECT_EQ(
      transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
      DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f));
}

TEST(DlTransformTest, DefaultConstructor) {
  DlTransform transform = DlTransform();

  EXPECT_TRUE(transform.IsIdentity());
  EXPECT_TRUE(transform.IsFinite());
  EXPECT_TRUE(transform.Is2D());
  EXPECT_TRUE(transform.IsTranslate());
  EXPECT_TRUE(transform.IsInvertible());
  EXPECT_TRUE(transform.IsScaleTranslate());
  EXPECT_TRUE(transform.RectStaysRect());
  EXPECT_FALSE(transform.HasPerspective());
  EXPECT_EQ(transform.Determinant(), kDlScalar_One);
  EXPECT_TRUE(CompareRowMajor(transform, kIdentityMatrix));
  EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(12.0f, 17.0f));
  EXPECT_EQ(
      transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
      DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f));
}

TEST(DlTransformTest, TranslateConstructor) {
  DlTransform transform = DlTransform::MakeTranslate(5.0f, 6.0f);

  EXPECT_FALSE(transform.IsIdentity());
  EXPECT_TRUE(transform.IsFinite());
  EXPECT_TRUE(transform.Is2D());
  EXPECT_TRUE(transform.IsTranslate());
  EXPECT_TRUE(transform.IsInvertible());
  EXPECT_TRUE(transform.IsScaleTranslate());
  EXPECT_TRUE(transform.RectStaysRect());
  EXPECT_FALSE(transform.HasPerspective());
  EXPECT_EQ(transform.Determinant(), kDlScalar_One);
  const DlScalar matrix[16] = {
      // clang-format off
      1.0f, 0.0f, 0.0f, 5.0f,
      0.0f, 1.0f, 0.0f, 6.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
      // clang-format on
  };
  EXPECT_TRUE(CompareRowMajor(transform, matrix));
  EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(17.0f, 23.0f));
  EXPECT_EQ(
      transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
      DlFRect::MakeLTRB(17.0f, 23.0f, 25.0f, 28.0f));
}

TEST(DlTransformTest, ScaleConstructor) {
  DlTransform transform = DlTransform::MakeScale(5.0f, 6.0f);

  EXPECT_FALSE(transform.IsIdentity());
  EXPECT_TRUE(transform.IsFinite());
  EXPECT_TRUE(transform.Is2D());
  EXPECT_FALSE(transform.IsTranslate());
  EXPECT_TRUE(transform.IsInvertible());
  EXPECT_TRUE(transform.IsScaleTranslate());
  EXPECT_TRUE(transform.RectStaysRect());
  EXPECT_FALSE(transform.HasPerspective());
  EXPECT_EQ(transform.Determinant(), 30.0f);
  const DlScalar matrix[16] = {
      // clang-format off
      5.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 6.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
      // clang-format on
  };
  EXPECT_TRUE(CompareRowMajor(transform, matrix));
  EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(60.0f, 102.0f));
  EXPECT_EQ(
      transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
      DlFRect::MakeLTRB(60.0f, 102.0f, 100.0f, 132.0f));
}

TEST(DlTransformTest, ConcatOrder) {
  DlTransform scale = DlTransform::MakeScale(5.0f, 6.0f);
  DlTransform translate = DlTransform::MakeTranslate(10.0f, 12.0f);

  const DlScalar matrix_scale_translate[16] = {
      // clang-format off
      5.0f, 0.0f, 0.0f, 50.0f,
      0.0f, 6.0f, 0.0f, 72.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
      // clang-format on
  };

  const DlScalar matrix_translate_scale[16] = {
      // clang-format off
      5.0f, 0.0f, 0.0f, 10.0f,
      0.0f, 6.0f, 0.0f, 12.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
      // clang-format on
  };

  {
    DlTransform transform = DlTransform::MakeConcat(scale, translate);
    EXPECT_TRUE(CompareRowMajor(transform, matrix_scale_translate));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(110.0f, 174.0f));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f),
              scale.TransformPoint(translate.TransformPoint(12.0f, 17.0f)));
    EXPECT_EQ(
        transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
        DlFRect::MakeLTRB(110.0f, 174.0f, 150.0f, 204.0f));
  }

  {
    DlTransform transform = DlTransform::MakeConcat(translate, scale);
    EXPECT_TRUE(CompareRowMajor(transform, matrix_translate_scale));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(70.0f, 114.0f));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f),
              translate.TransformPoint(scale.TransformPoint(12.0f, 17.0f)));
    EXPECT_EQ(
        transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
        DlFRect::MakeLTRB(70.0f, 114.0f, 110.0f, 144.0f));
  }

  {
    DlTransform transform;
    transform.ConcatInner(scale);
    transform.ConcatInner(translate);
    EXPECT_TRUE(CompareRowMajor(transform, matrix_scale_translate));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(110.0f, 174.0f));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f),
              scale.TransformPoint(translate.TransformPoint(12.0f, 17.0f)));
    EXPECT_EQ(
        transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
        DlFRect::MakeLTRB(110.0f, 174.0f, 150.0f, 204.0f));
  }

  {
    DlTransform transform;
    transform.ConcatOuter(scale);
    transform.ConcatOuter(translate);
    EXPECT_TRUE(CompareRowMajor(transform, matrix_translate_scale));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f), DlFPoint(70.0f, 114.0f));
    EXPECT_EQ(transform.TransformPoint(12.0f, 17.0f),
              translate.TransformPoint(scale.TransformPoint(12.0f, 17.0f)));
    EXPECT_EQ(
        transform.TransformRect(DlFRect::MakeLTRB(12.0f, 17.0f, 20.0f, 22.0f)),
        DlFRect::MakeLTRB(70.0f, 114.0f, 110.0f, 144.0f));
  }
}

TEST(DlTransformTest, Concat) {
  auto perspX = DlTransform::MakeRowMajor(
      // clang-format off
      1.0f,   0.0f, 0.0f, 0.0f,
      0.0f,   1.0f, 0.0f, 0.0f,
      0.0f,   0.0f, 1.0f, 0.0f,
      0.001f, 0.0f, 0.0f, 1.0f
      // clang-format on
  );
  auto perspY = DlTransform::MakeRowMajor(
      // clang-format off
      1.0f, 0.0f,   0.0f, 0.0f,
      0.0f, 1.0f,   0.0f, 0.0f,
      0.0f, 0.0f,   1.0f, 0.0f,
      0.0f, 0.001f, 0.0f, 1.0f
      // clang-format on
  );
  std::vector<DlTransform> transforms = {
      DlTransform(),
      DlTransform::MakeTranslate(8.0f, 11.0f),
      DlTransform::MakeScale(2.0f, 4.0f),
      DlTransform::MakeSkew(0.125f, 0.25f),
      DlTransform::MakeRotate(DlDegrees(30)),
      perspX,
      perspY,
  };
  std::vector<DlFPoint> points = {
      DlFPoint(0, 0),
      DlFPoint(10, 11),
      DlFPoint(1.0e5f, 12),
      DlFPoint(16, 1.0e5f),
  };
  auto test = [points](const DlTransform& outer,  //
                       const DlTransform& inner,  //
                       const std::string& desc) {
    DlTransform concat = DlTransform::MakeConcat(outer, inner);
    for (size_t i = 0; i < points.size(); i++) {
      EXPECT_TRUE(
          IsClose(concat.TransformPoint(points[i]),
                  outer.TransformPoint(inner.TransformPoint(points[i]))))
          << desc << ", point#" << (i + 1) << " = " << points[i] << std::endl
          << "outer=" << outer << std::endl
          << "inner=" << inner << std::endl
          << "concat=" << concat;
    }
  };
  for (size_t i = 0; i < transforms.size(); i++) {
    for (size_t j = 0; j < transforms.size(); j++) {
      std::string desc =
          "tx#" + std::to_string(i + 1) + " X tx#" + std::to_string(j + 1);
      test(transforms[i], transforms[j], desc);
    }
  }
}

TEST(DlTransformTest, Inverse) {
  auto perspX = DlTransform::MakeRowMajor(
      // clang-format off
      1.0f,   0.0f, 0.0f, 0.0f,
      0.0f,   1.0f, 0.0f, 0.0f,
      0.0f,   0.0f, 1.0f, 0.0f,
      0.001f, 0.0f, 0.0f, 1.0f
      // clang-format on
  );
  auto perspY = DlTransform::MakeRowMajor(
      // clang-format off
      1.0f, 0.0f,   0.0f, 0.0f,
      0.0f, 1.0f,   0.0f, 0.0f,
      0.0f, 0.0f,   1.0f, 0.0f,
      0.0f, 0.001f, 0.0f, 1.0f
      // clang-format on
  );
  std::vector<DlTransform> transforms = {
      DlTransform(),
      DlTransform::MakeTranslate(8.0f, 11.0f),
      DlTransform::MakeScale(2.0f, 4.0f),
      DlTransform::MakeSkew(0.125f, 0.25f),
      DlTransform::MakeRotate(DlDegrees(45)),
      perspX,
      perspY,
  };
  for (size_t i = 0; i < transforms.size(); i++) {
    std::string desc1 = "tx#" + std::to_string(i + 1);
    DlTransform transform1 = transforms[i];
    auto inverse1 = transform1.Inverse();
    ENFORCE_TRUE(inverse1.has_value());
    EXPECT_TRUE(IsClose(DlTransform::MakeConcat(transform1, inverse1.value()),
                        DlTransform()))
        << desc1;
    EXPECT_TRUE(
        IsClose(DlTransform::MakeConcat(*inverse1, transform1), DlTransform()))
        << desc1;
    for (size_t j = 0; j < transforms.size(); j++) {
      std::string desc2 = desc1 + " X tx#" + std::to_string(j + 1);
      DlTransform transform2 =
          DlTransform::MakeConcat(transform1, transforms[j]);
      auto inverse2 = transform2.Inverse();
      ENFORCE_TRUE(inverse2.has_value());
      EXPECT_TRUE(IsClose(DlTransform::MakeConcat(transform2, *inverse2),
                          DlTransform()))
          << desc2;
      EXPECT_TRUE(IsClose(DlTransform::MakeConcat(*inverse2, transform2),
                          DlTransform()))
          << desc2;
    }
  }
}

static constexpr int kCanTestSkMatrix = 1;
static constexpr int kCanTestSkM44 = 2;
static_assert((kCanTestSkMatrix & kCanTestSkM44) == 0);

struct TransformSetup {
  std::string name;
  int can_test;
  std::function<DlTransform()> DlCreate;
  std::function<SkMatrix()> SkCreate;
  std::function<SkM44()> Sk44Create;
  std::function<void(DlTransform& transform)> DlSet;
  std::function<void(SkMatrix& transform)> SkSet;
  std::function<void(SkM44& transform)> Sk44Set;
  std::function<void(DlTransform& transform)> DlApplyInner;
  std::function<void(SkMatrix& transform)> SkApplyInner;
  std::function<void(SkM44& transform)> Sk44ApplyInner;
  std::function<void(DlTransform& transform)> DlApplyOuter;
  std::function<void(SkMatrix& transform)> SkApplyOuter;
  std::function<void(SkM44& transform)> Sk44ApplyOuter;
};

static void TestChain(const std::vector<TransformSetup*>& setup_chain,
                      DlTransform dlt,
                      const std::string& desc) {
  std::vector<DlFPoint> points = {
      DlFPoint(0, 0),
      DlFPoint(10, 11),
      DlFPoint(1.0e5f, 12),
      DlFPoint(16, 1.0e5f),
  };
  int can_test = kCanTestSkMatrix | kCanTestSkM44;
  for (auto& setup : setup_chain) {
    can_test &= setup->can_test;
  }
  FML_DCHECK(can_test != 0);
  for (DlFPoint& p : points) {
    auto dl_result = dlt.TransformPoint(p);
    DlFHomogenous3D dl_expected = DlFHomogenous3D(p);
    for (auto& setup : setup_chain) {
      dl_expected = setup->DlCreate().TransformHomogenous(dl_expected);
    }
    EXPECT_TRUE(IsClose(dl_result, dl_expected.NormalizeToPoint()))
        << desc << " (DL)";

    if (can_test & kCanTestSkMatrix) {
      SkPoint sk_expected = SkPoint::Make(p.x(), p.y());
      for (auto& setup : setup_chain) {
        sk_expected = setup->SkCreate().mapPoint(sk_expected);
      }
      EXPECT_TRUE(IsClose(dl_result, sk_expected)) << desc << " (SkMatrix)";
    }

    if (can_test & kCanTestSkM44) {
      SkV4 sk4_expected = {p.x(), p.y(), 0, 1};
      for (auto& setup : setup_chain) {
        sk4_expected = setup->Sk44Create() * sk4_expected;
      }
      EXPECT_TRUE(IsClose(dl_result, sk4_expected)) << desc << " (SkM44)";
    }
  }
}

TEST(DlTransformTest, CompareToSkia) {
  TransformSetup setups[] = {
      {
          "Identity",
          kCanTestSkMatrix | kCanTestSkM44,
          []() { return DlTransform(); },
          []() { return SkMatrix(); },
          []() { return SkM44(); },
          [](DlTransform& transform) { transform.SetIdentity(); },
          [](SkMatrix& transform) { transform.setIdentity(); },
          [](SkM44& transform) { transform.setIdentity(); },
          [](DlTransform& transform) {},
          [](SkMatrix& transform) {},
          [](SkM44& transform) {},
          [](DlTransform& transform) {},
          [](SkMatrix& transform) {},
          [](SkM44& transform) {},
      },
      {
          "Translate(5, 10)",
          kCanTestSkMatrix | kCanTestSkM44,
          []() { return DlTransform::MakeTranslate(5, 10); },
          []() { return SkMatrix::Translate(5, 10); },
          []() { return SkM44::Translate(5, 10); },
          [](DlTransform& transform) { transform.SetTranslate(5, 10); },
          [](SkMatrix& transform) { transform.setTranslate(5, 10); },
          [](SkM44& transform) { transform.setTranslate(5, 10); },
          [](DlTransform& transform) { transform.TranslateInner(5, 10); },
          [](SkMatrix& transform) { transform.preTranslate(5, 10); },
          [](SkM44& transform) { transform.preTranslate(5, 10); },
          [](DlTransform& transform) { transform.TranslateOuter(5, 10); },
          [](SkMatrix& transform) { transform.postTranslate(5, 10); },
          [](SkM44& transform) { transform.postTranslate(5, 10); },
      },
      {
          "Scale(5, 10)",
          kCanTestSkMatrix | kCanTestSkM44,
          []() { return DlTransform::MakeScale(5, 10); },
          []() { return SkMatrix::Scale(5, 10); },
          []() { return SkM44::Scale(5, 10); },
          [](DlTransform& transform) { transform.SetScale(5, 10); },
          [](SkMatrix& transform) { transform.setScale(5, 10); },
          [](SkM44& transform) { transform.setScale(5, 10); },
          [](DlTransform& transform) { transform.ScaleInner(5, 10); },
          [](SkMatrix& transform) { transform.preScale(5, 10); },
          [](SkM44& transform) { transform.preScale(5, 10); },
          [](DlTransform& transform) { transform.ScaleOuter(5, 10); },
          [](SkMatrix& transform) { transform.postScale(5, 10); },
          [](SkM44& transform) { transform.postConcat(SkM44::Scale(5, 10)); },
      },
      {
          // Default rotate methods on DlTransform which assume Z axis
          "Rotate(20 degrees)",
          kCanTestSkMatrix | kCanTestSkM44,
          []() { return DlTransform::MakeRotate(DlDegrees(20)); },
          []() { return SkMatrix::RotateDeg(20); },
          []() {
            return SkM44::Rotate({0, 0, 1}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) { transform.SetRotate(DlDegrees(20)); },
          [](SkMatrix& transform) { transform.setRotate(20); },
          [](SkM44& transform) {
            transform.setRotate({0, 0, 1}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) { transform.RotateInner(DlDegrees(20)); },
          [](SkMatrix& transform) { transform.preRotate(20); },
          [](SkM44& transform) {
            transform.preConcat(
                SkM44::Rotate({0, 0, 1}, 20 * SK_ScalarPI / 180));
          },
          [](DlTransform& transform) { transform.RotateOuter(DlDegrees(20)); },
          [](SkMatrix& transform) { transform.postRotate(20); },
          [](SkM44& transform) {
            transform.postConcat(
                SkM44::Rotate({0, 0, 1}, 20 * SK_ScalarPI / 180));
          },
      },
      {
          // Rotate methods on DlTransform which explicitly specify Z axis
          "RotateZ(20 degrees)",
          kCanTestSkMatrix | kCanTestSkM44,
          []() { return DlTransform::MakeRotate(kDlAxis_Z, DlDegrees(20)); },
          []() { return SkMatrix::RotateDeg(20); },
          []() {
            return SkM44::Rotate({0, 0, 1}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) {
            transform.SetRotate(kDlAxis_Z, DlDegrees(20));
          },
          [](SkMatrix& transform) { transform.setRotate(20); },
          [](SkM44& transform) {
            transform.setRotate({0, 0, 1}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) {
            transform.RotateInner(kDlAxis_Z, DlDegrees(20));
          },
          [](SkMatrix& transform) { transform.preRotate(20); },
          [](SkM44& transform) {
            transform.preConcat(
                SkM44::Rotate({0, 0, 1}, 20 * SK_ScalarPI / 180));
          },
          [](DlTransform& transform) {
            transform.RotateOuter(kDlAxis_Z, DlDegrees(20));
          },
          [](SkMatrix& transform) { transform.postRotate(20); },
          [](SkM44& transform) {
            transform.postConcat(
                SkM44::Rotate({0, 0, 1}, 20 * SK_ScalarPI / 180));
          },
      },
      {
          "RotateX(20 degrees)",
          kCanTestSkM44,
          []() { return DlTransform::MakeRotate(kDlAxis_X, DlDegrees(20)); },
          []() { return SkMatrix(); },  // Not executed
          []() {
            return SkM44::Rotate({1, 0, 0}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) {
            transform.SetRotate(kDlAxis_X, DlDegrees(20));
          },
          [](SkMatrix& transform) {},  // Not executed
          [](SkM44& transform) {
            transform.setRotate({1, 0, 0}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) {
            transform.RotateInner(kDlAxis_X, DlDegrees(20));
          },
          [](SkMatrix& transform) {},  // Not executed
          [](SkM44& transform) {
            transform.preConcat(
                SkM44::Rotate({1, 0, 0}, 20 * SK_ScalarPI / 180));
          },
          [](DlTransform& transform) {
            transform.RotateOuter(kDlAxis_X, DlDegrees(20));
          },
          [](SkMatrix& transform) {},  // Not executed
          [](SkM44& transform) {
            transform.postConcat(
                SkM44::Rotate({1, 0, 0}, 20 * SK_ScalarPI / 180));
          },
      },
      {
          "RotateY(20 degrees)",
          kCanTestSkM44,
          []() { return DlTransform::MakeRotate(kDlAxis_Y, DlDegrees(20)); },
          []() { return SkMatrix(); },  // Not executed
          []() {
            return SkM44::Rotate({0, 1, 0}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) {
            transform.SetRotate(kDlAxis_Y, DlDegrees(20));
          },
          [](SkMatrix& transform) {},  // Not executed
          [](SkM44& transform) {
            transform.setRotate({0, 1, 0}, 20 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) {
            transform.RotateInner(kDlAxis_Y, DlDegrees(20));
          },
          [](SkMatrix& transform) {},  // Not executed
          [](SkM44& transform) {
            transform.preConcat(
                SkM44::Rotate({0, 1, 0}, 20 * SK_ScalarPI / 180));
          },
          [](DlTransform& transform) {
            transform.RotateOuter(kDlAxis_Y, DlDegrees(20));
          },
          [](SkMatrix& transform) {},  // Not executed
          [](SkM44& transform) {
            transform.postConcat(
                SkM44::Rotate({0, 1, 0}, 20 * SK_ScalarPI / 180));
          },
      },
      {
          "RotateZ(200 degrees)",
          kCanTestSkMatrix | kCanTestSkM44,
          []() { return DlTransform::MakeRotate(DlDegrees(200)); },
          []() { return SkMatrix::RotateDeg(200); },
          []() {
            return SkM44::Rotate({0, 0, 1}, 200 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) { transform.SetRotate(DlDegrees(200)); },
          [](SkMatrix& transform) { transform.setRotate(200); },
          [](SkM44& transform) {
            transform.setRotate({0, 0, 1}, 200 * SK_ScalarPI / 180);
          },
          [](DlTransform& transform) { transform.RotateInner(DlDegrees(200)); },
          [](SkMatrix& transform) { transform.preRotate(200); },
          [](SkM44& transform) {
            transform.preConcat(
                SkM44::Rotate({0, 0, 1}, 200 * SK_ScalarPI / 180));
          },
          [](DlTransform& transform) { transform.RotateOuter(DlDegrees(200)); },
          [](SkMatrix& transform) { transform.postRotate(200); },
          [](SkM44& transform) {
            transform.postConcat(
                SkM44::Rotate({0, 0, 1}, 200 * SK_ScalarPI / 180));
          },
      },
  };
  int count = sizeof(setups) / sizeof(setups[0]);
  for (int i = 0; i < count; i++) {
    TransformSetup& setup1 = setups[i];
    std::string desc = setup1.name;
    int can_test = setup1.can_test;
    {
      DlTransform dlt = setup1.DlCreate();
      if (can_test & kCanTestSkMatrix) {
        SkMatrix skt = setup1.SkCreate();
        EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
      }
      if (can_test & kCanTestSkM44) {
        SkM44 sk4t = setup1.Sk44Create();
        EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
      }
    }
    {
      DlTransform dlt;
      setup1.DlSet(dlt);
      if (can_test & kCanTestSkMatrix) {
        SkMatrix skt;
        setup1.SkSet(skt);
        EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
      }
      if (can_test & kCanTestSkM44) {
        SkM44 sk4t;
        setup1.Sk44Set(sk4t);
        EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
      }
    }
    {
      DlTransform dlt;
      setup1.DlApplyInner(dlt);
      if (can_test & kCanTestSkMatrix) {
        SkMatrix skt;
        setup1.SkApplyInner(skt);
        EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
      }
      if (can_test & kCanTestSkM44) {
        SkM44 sk4t;
        setup1.Sk44ApplyInner(sk4t);
        EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
      }
    }
    {
      DlTransform dlt;
      setup1.DlApplyOuter(dlt);
      if (can_test & kCanTestSkMatrix) {
        SkMatrix skt;
        setup1.SkApplyOuter(skt);
        EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
      }
      if (can_test & kCanTestSkM44) {
        SkM44 sk4t;
        setup1.Sk44ApplyOuter(sk4t);
        EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
      }
    }
    TestChain({&setup1}, setup1.DlCreate(), desc);
    for (int j = 0; j < count; j++) {
      TransformSetup setup2 = setups[j];
      desc = setup1.name + ", " + setup2.name;
      can_test = setup1.can_test & setup2.can_test;
      FML_DCHECK(can_test != 0);
      {
        DlTransform dlt;
        setup1.DlApplyInner(dlt);
        setup2.DlApplyInner(dlt);
        if (can_test & kCanTestSkMatrix) {
          SkMatrix skt;
          setup1.SkApplyInner(skt);
          setup2.SkApplyInner(skt);
          EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
        }
        if (can_test & kCanTestSkM44) {
          SkM44 sk4t;
          setup1.Sk44ApplyInner(sk4t);
          setup2.Sk44ApplyInner(sk4t);
          EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
        }
        TestChain({&setup2, &setup1}, dlt, desc);
      }
      {
        DlTransform dlt;
        setup1.DlApplyOuter(dlt);
        setup2.DlApplyOuter(dlt);
        if (can_test & kCanTestSkMatrix) {
          SkMatrix skt;
          setup1.SkApplyOuter(skt);
          setup2.SkApplyOuter(skt);
          EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
        }
        if (can_test & kCanTestSkM44) {
          SkM44 sk4t;
          setup1.Sk44ApplyOuter(sk4t);
          setup2.Sk44ApplyOuter(sk4t);
          EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
        }
        TestChain({&setup1, &setup2}, dlt, desc);
      }
      for (int k = 0; k < count; k++) {
        TransformSetup setup3 = setups[k];
        desc = setup1.name + ", " + setup2.name + ", " + setup3.name;
        can_test = setup1.can_test & setup2.can_test & setup3.can_test;
        {
          DlTransform dlt;
          setup1.DlApplyInner(dlt);
          setup2.DlApplyInner(dlt);
          setup3.DlApplyInner(dlt);
          if (can_test & kCanTestSkMatrix) {
            SkMatrix skt;
            setup1.SkApplyInner(skt);
            setup2.SkApplyInner(skt);
            setup3.SkApplyInner(skt);
            EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
          }
          if (can_test & kCanTestSkM44) {
            SkM44 sk4t;
            setup1.Sk44ApplyInner(sk4t);
            setup2.Sk44ApplyInner(sk4t);
            setup3.Sk44ApplyInner(sk4t);
            EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
          }
          TestChain({&setup3, &setup2, &setup1}, dlt, desc);
        }
        {
          DlTransform dlt;
          setup1.DlApplyOuter(dlt);
          setup2.DlApplyOuter(dlt);
          setup3.DlApplyOuter(dlt);
          if (can_test & kCanTestSkMatrix) {
            SkMatrix skt;
            setup1.SkApplyOuter(skt);
            setup2.SkApplyOuter(skt);
            setup3.SkApplyOuter(skt);
            EXPECT_TRUE(IsClose(dlt, skt)) << desc << " (SkMatrix)";
          }
          if (can_test & kCanTestSkM44) {
            SkM44 sk4t;
            setup1.Sk44ApplyOuter(sk4t);
            setup2.Sk44ApplyOuter(sk4t);
            setup3.Sk44ApplyOuter(sk4t);
            EXPECT_TRUE(IsClose(dlt, sk4t)) << desc << " (SkM44)";
          }
          TestChain({&setup1, &setup2, &setup3}, dlt, desc);
        }
      }
    }
  }
}

}  // namespace testing
}  // namespace flutter
