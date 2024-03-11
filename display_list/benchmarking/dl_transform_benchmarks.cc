// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/benchmarking/benchmarking.h"

#include "flutter/impeller/geometry/matrix.h"
#include "flutter/impeller/geometry/rect.h"
#include "third_party/skia/include/core/SkM44.h"
#include "third_party/skia/include/core/SkMatrix.h"

namespace flutter {

namespace {

static constexpr float kPiOver4 = impeller::kPiOver4;

enum class AdapterType {
  kSkMatrix,
  kSkM44,
  kImpellerMatrix,
};

// This struct should be a "castable" replacement for the various rect
// objects being used (SkRect or impeller::Rect), so that no adapter
// gets an advantage in the rect bounds transform benchmarks. Each
// sub-class of Adapter (Skia vs Impeller) will assert this similarity
// in its constructor (which is only executed once per benchmark in the
// setup code outside of the timed body).
struct TestRect {
  static constexpr TestRect MakeLTRB(float left,
                                     float top,
                                     float right,
                                     float bottom) {
    return TestRect(left, top, right, bottom);
  }

  float left;
  float top;
  float right;
  float bottom;

 private:
  constexpr TestRect(float l, float t, float r, float b)
      : left(l), top(t), right(r), bottom(b) {}
};

// We use a virtual adapter class rather than templating the BM_* methods
// to prevent the compiler from optimizing the benchmark bodies into a
// null set of instructions because the calculation can be proven to have
// no side effects and the result is never used.
class TransformAdapter {
 public:
  TransformAdapter() = default;
  virtual ~TransformAdapter() = default;

  // Two methods to test the overhead of just calling a virtual method on
  // the adapter (should be the same for all inheriting subclasses) and
  // for a method that does a conversion to and from the TestRect object
  // (which should be the same as the method call overhead since it does
  // no work).
  virtual void DoNothing() const = 0;
  virtual TestRect DoNothing(const TestRect& rect) const = 0;

  // The actual methods that do work and are the meat of the benchmarks.
  virtual void SetIdentity() = 0;
  virtual void SetTranslate(float tx, float ty) = 0;
  virtual void SetScale(float sx, float sy) = 0;
  virtual void SetRotate(float radians) = 0;

  virtual void Translate(float tx, float ty) = 0;
  virtual void Scale(float sx, float sy) = 0;
  virtual void Rotate(float radians) = 0;

  virtual TestRect TransformRect(const TestRect& rect) const = 0;
};

class SkiaAdapterBase : public TransformAdapter {
 public:
  SkiaAdapterBase() {
    static_assert(sizeof(TestRect) == sizeof(SkRect));
    const TestRect tr = TestRect::MakeLTRB(1, 2, 3, 4);
    auto& sr = ToSkRect(tr);
    FML_CHECK(sr.fLeft == 1.0f);
    FML_CHECK(sr.fTop == 2.0f);
    FML_CHECK(sr.fRight == 3.0f);
    FML_CHECK(sr.fBottom == 4.0f);
  }

  // DoNothing methods used to measure overhead for various operations
  void DoNothing() const override {}
  TestRect DoNothing(const TestRect& rect) const override {
    return ToTestRect(ToSkRect(rect));
  }

 protected:
  static constexpr const SkRect& ToSkRect(const TestRect& rect) {
    return *((const SkRect*)(&rect));
  }

  static constexpr const TestRect& ToTestRect(const SkRect& rect) {
    return *((const TestRect*)(&rect));
  }
};

class SkMatrixAdapter : public SkiaAdapterBase {
 public:
  SkMatrixAdapter() = default;
  ~SkMatrixAdapter() = default;

  void SetIdentity() override { transform_ = SkMatrix::I(); }

  void SetTranslate(float tx, float ty) override {
    transform_ = SkMatrix::Translate(tx, ty);
  }

  void SetScale(float sx, float sy) override {
    transform_ = SkMatrix::Scale(sx, sy);
  }

  void SetRotate(float radians) override {
    transform_ = SkMatrix::RotateRad(radians);
  }

  void Translate(float tx, float ty) override {
    transform_.preTranslate(tx, ty);
  }

  void Scale(float sx, float sy) override { transform_.preScale(sx, sy); }

  void Rotate(float radians) override {
    transform_.preRotate(SkRadiansToDegrees(radians));
  }

  TestRect TransformRect(const TestRect& rect) const override {
    return ToTestRect(transform_.mapRect(ToSkRect(rect)));
  }

 private:
  SkMatrix transform_;
};

class SkM44Adapter : public SkiaAdapterBase {
 public:
  SkM44Adapter() = default;
  ~SkM44Adapter() = default;

  void SetIdentity() override { transform_ = SkM44(); }

  void SetTranslate(float tx, float ty) override {
    transform_ = SkM44::Translate(tx, ty);
  }

  void SetScale(float sx, float sy) override {
    transform_ = SkM44::Scale(sx, sy);
  }

  void SetRotate(float radians) override {
    transform_ = SkM44::Rotate({0, 0, 1}, radians);
  }

  void Translate(float tx, float ty) override {
    transform_.preTranslate(tx, ty);
  }

  void Scale(float sx, float sy) override { transform_.preScale(sx, sy); }

  void Rotate(float radians) override {
    transform_.preConcat(SkM44::Rotate({0, 0, 1}, radians));
  }

  TestRect TransformRect(const TestRect& rect) const override {
    return ToTestRect(transform_.asM33().mapRect(ToSkRect(rect)));
  }

 private:
  SkM44 transform_;
};

class ImpellerMatrixAdapter : public TransformAdapter {
 public:
  ImpellerMatrixAdapter() {
    static_assert(sizeof(TestRect) == sizeof(impeller::Rect));
    const TestRect tr = TestRect::MakeLTRB(1, 2, 3, 4);
    auto& sr = ToImpellerRect(tr);
    FML_CHECK(sr.GetLeft() == 1.0f);
    FML_CHECK(sr.GetTop() == 2.0f);
    FML_CHECK(sr.GetRight() == 3.0f);
    FML_CHECK(sr.GetBottom() == 4.0f);
  }

  ~ImpellerMatrixAdapter() = default;

  void DoNothing() const override {}
  TestRect DoNothing(const TestRect& rect) const override {
    return ToTestRect(ToImpellerRect(rect));
  }

  void SetIdentity() override { transform_ = impeller::Matrix(); }

  void SetTranslate(float tx, float ty) override {
    transform_ = impeller::Matrix::MakeTranslation({tx, ty});
  }

  void SetScale(float sx, float sy) override {
    transform_ = impeller::Matrix::MakeScale({sx, sy, 1.0f});
  }

  void SetRotate(float radians) override {
    transform_ = impeller::Matrix::MakeRotationZ(impeller::Radians(radians));
  }

  void Translate(float tx, float ty) override {
    transform_ = transform_.Translate({tx, ty});
  }

  void Scale(float sx, float sy) override {
    transform_ = transform_.Scale({sx, sy, 1.0f});
  }

  void Rotate(float radians) override {
    transform_ = transform_ *
                 impeller::Matrix::MakeRotationZ(impeller::Radians(radians));
  }

  TestRect TransformRect(const TestRect& rect) const override {
    return ToTestRect(ToImpellerRect(rect).TransformBounds(transform_));
  }

 private:
  impeller::Matrix transform_;

  static constexpr const impeller::Rect& ToImpellerRect(const TestRect& rect) {
    return *((const impeller::Rect*)(&rect));
  }

  static constexpr const TestRect& ToTestRect(const impeller::Rect& rect) {
    return *((const TestRect*)(&rect));
  }
};

// We use a function to return the appropriate adapter so that all methods
// used in benchmarking are "pure virtual" and cannot be optimized out
// due to issues such as the arguments being constexpr and the result
// simplified to a constant value.
static std::unique_ptr<TransformAdapter> GetAdapter(AdapterType type) {
  switch (type) {
    case AdapterType::kSkMatrix:
      return std::make_unique<SkMatrixAdapter>();
    case AdapterType::kSkM44:
      return std::make_unique<SkM44Adapter>();
    case AdapterType::kImpellerMatrix:
      return std::make_unique<ImpellerMatrixAdapter>();
  }
  FML_UNREACHABLE();
}

}  // namespace

static void BM_AdapterDispatchOverhead(benchmark::State& state,
                                       AdapterType type) {
  auto tx = GetAdapter(type);
  while (state.KeepRunning()) {
    tx->DoNothing();
  }
}

static void BM_AdapterRectOverhead(benchmark::State& state, AdapterType type) {
  auto tx = GetAdapter(type);
  TestRect rect = TestRect::MakeLTRB(100, 100, 200, 200);
  while (state.KeepRunning()) {
    tx->DoNothing(rect);
  }
}

static void BM_SetIdentity(benchmark::State& state, AdapterType type) {
  auto tx = GetAdapter(type);
  while (state.KeepRunning()) {
    tx->SetIdentity();
  }
}

static void BM_SetTranslate(benchmark::State& state,
                            AdapterType type,
                            float x,
                            float y) {
  auto tx = GetAdapter(type);
  while (state.KeepRunning()) {
    tx->SetTranslate(x, y);
  }
}

static void BM_SetScale(benchmark::State& state,
                        AdapterType type,
                        float scale) {
  auto tx = GetAdapter(type);
  while (state.KeepRunning()) {
    tx->SetScale(scale, scale);
  }
}

static void BM_SetRotate(benchmark::State& state,
                         AdapterType type,
                         float radians) {
  auto tx = GetAdapter(type);
  while (state.KeepRunning()) {
    tx->SetRotate(radians);
  }
}

static void BM_IdentityBounds(benchmark::State& state, AdapterType type) {
  auto tx = GetAdapter(type);
  TestRect rect = TestRect::MakeLTRB(100, 100, 200, 200);
  while (state.KeepRunning()) {
    tx->TransformRect(rect);
  }
}

static void BM_TranslateBounds(benchmark::State& state,
                               AdapterType type,
                               float x,
                               float y) {
  auto tx = GetAdapter(type);
  tx->Translate(x, y);
  TestRect rect = TestRect::MakeLTRB(100, 100, 200, 200);
  while (state.KeepRunning()) {
    tx->TransformRect(rect);
  }
}

static void BM_ScaleBounds(benchmark::State& state,
                           AdapterType type,
                           float scale) {
  auto tx = GetAdapter(type);
  TestRect rect = TestRect::MakeLTRB(100, 100, 200, 200);
  tx->SetScale(scale, scale);
  while (state.KeepRunning()) {
    tx->TransformRect(rect);
  }
}

static void BM_ScaleTranslateBounds(benchmark::State& state,
                                    AdapterType type,
                                    float scale,
                                    float x,
                                    float y) {
  auto tx = GetAdapter(type);
  tx->SetScale(scale, scale);
  tx->Translate(x, y);
  TestRect rect = TestRect::MakeLTRB(100, 100, 200, 200);
  while (state.KeepRunning()) {
    tx->TransformRect(rect);
  }
}

static void BM_RotateBounds(benchmark::State& state,
                            AdapterType type,
                            float radians) {
  auto tx = GetAdapter(type);
  tx->SetRotate(radians);
  TestRect rect = TestRect::MakeLTRB(100, 100, 200, 200);
  while (state.KeepRunning()) {
    tx->TransformRect(rect);
  }
}

#define BENCHMARK_CAPTURE_TYPE(name, type) \
  BENCHMARK_CAPTURE(name, type, AdapterType::k##type)

#define BENCHMARK_CAPTURE_TYPE_ARGS(name, type, ...) \
  BENCHMARK_CAPTURE(name, type, AdapterType::k##type, __VA_ARGS__)

#define BENCHMARK_CAPTURE_ALL(name)       \
  BENCHMARK_CAPTURE_TYPE(name, SkMatrix); \
  BENCHMARK_CAPTURE_TYPE(name, SkM44);    \
  BENCHMARK_CAPTURE_TYPE(name, ImpellerMatrix)

#define BENCHMARK_CAPTURE_ALL_ARGS(name, ...)               \
  BENCHMARK_CAPTURE_TYPE_ARGS(name, SkMatrix, __VA_ARGS__); \
  BENCHMARK_CAPTURE_TYPE_ARGS(name, SkM44, __VA_ARGS__);    \
  BENCHMARK_CAPTURE_TYPE_ARGS(name, ImpellerMatrix, __VA_ARGS__)

BENCHMARK_CAPTURE_ALL(BM_AdapterDispatchOverhead);
BENCHMARK_CAPTURE_ALL(BM_AdapterRectOverhead);

BENCHMARK_CAPTURE_ALL(BM_SetIdentity);
BENCHMARK_CAPTURE_ALL_ARGS(BM_SetTranslate, 10.0f, 15.0f);
BENCHMARK_CAPTURE_ALL_ARGS(BM_SetScale, 2.0f);
BENCHMARK_CAPTURE_ALL_ARGS(BM_SetRotate, kPiOver4);

BENCHMARK_CAPTURE_ALL(BM_IdentityBounds);
BENCHMARK_CAPTURE_ALL_ARGS(BM_TranslateBounds, 10.0f, 15.0f);
BENCHMARK_CAPTURE_ALL_ARGS(BM_ScaleBounds, 2.0f);
BENCHMARK_CAPTURE_ALL_ARGS(BM_ScaleTranslateBounds, 2.0f, 10.0f, 15.0f);
BENCHMARK_CAPTURE_ALL_ARGS(BM_RotateBounds, kPiOver4);

}  // namespace flutter
