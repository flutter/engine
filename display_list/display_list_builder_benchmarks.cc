// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/benchmarking/benchmarking.h"
#include "flutter/display_list/display_list_test_utils.h"

namespace flutter {
namespace {

const static int kDefault = 0;
const static int kBounds = 1;
const static int kBoundsAndRtree = 2;

static void InvokeAllRenderingOps(DisplayListBuilder& builder) {
  for (auto& group : testing::allRenderingOps) {
    for (size_t i = 0; i < group.variants.size(); i++) {
      auto& invocation = group.variants[i];
      invocation.Invoke(builder);
    }
  }
}

static void Complete(DisplayListBuilder& builder, int type) {
  auto display_list = builder.Build();
  switch (type) {
    case kBounds:
      display_list->bounds();
      break;
    case kBoundsAndRtree:
      display_list->bounds();
      display_list->rtree();
      break;
    default:
      break;
  }
}

}  // namespace

static void BM_DisplayListBuiderDefault(benchmark::State& state) {
  int type = state.range(0);
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    InvokeAllRenderingOps(builder);
    Complete(builder, type);
  }
}

static void BM_DisplayListBuiderWithScaleAndTranslate(benchmark::State& state) {
  int type = state.range(0);
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    builder.scale(3.5, 3.5);
    builder.translate(10.3, 6.9);
    InvokeAllRenderingOps(builder);
    Complete(builder, type);
  }
}

static void BM_DisplayListBuiderWithPerspective(benchmark::State& state) {
  int type = state.range(0);
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    builder.transformFullPerspective(0, 1, 0, 12, 1, 0, 0, 33, 3, 2, 5, 29, 0,
                                     0, 0, 12);
    InvokeAllRenderingOps(builder);
    Complete(builder, type);
  }
}

static void BM_DisplayListBuiderWithClipRect(benchmark::State& state) {
  int type = state.range(0);
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    SkRect clip_bounds = SkRect::MakeLTRB(6.5, 7.3, 90.2, 85.7);
    builder.clipRect(clip_bounds, SkClipOp::kIntersect, true);
    InvokeAllRenderingOps(builder);
    Complete(builder, type);
  }
}

static void BM_DisplayListBuiderWithSaveLayer(benchmark::State& state) {
  int type = state.range(0);
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    for (auto& group : testing::allRenderingOps) {
      for (size_t i = 0; i < group.variants.size(); i++) {
        auto& invocation = group.variants[i];
        builder.saveLayer(nullptr, false);
        invocation.Invoke(builder);
        builder.restore();
      }
    }
    Complete(builder, type);
  }
}

static void BM_DisplayListBuiderWithSaveLayerAndImageFilter(
    benchmark::State& state) {
  int type = state.range(0);
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    for (auto& group : testing::allRenderingOps) {
      for (size_t i = 0; i < group.variants.size(); i++) {
        auto& invocation = group.variants[i];
        builder.saveLayer(nullptr, SaveLayerOptions::kNoAttributes,
                          &testing::kTestBlurImageFilter1);
        invocation.Invoke(builder);
        builder.restore();
      }
    }
    Complete(builder, type);
  }
}

BENCHMARK(BM_DisplayListBuiderDefault)
    ->Args({kDefault})
    ->Args({kBounds})
    ->Args({kBoundsAndRtree})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_DisplayListBuiderWithScaleAndTranslate)
    ->Args({kDefault})
    ->Args({kBounds})
    ->Args({kBoundsAndRtree})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_DisplayListBuiderWithPerspective)
    ->Args({kDefault})
    ->Args({kBounds})
    ->Args({kBoundsAndRtree})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_DisplayListBuiderWithClipRect)
    ->Args({kDefault})
    ->Args({kBounds})
    ->Args({kBoundsAndRtree})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_DisplayListBuiderWithSaveLayer)
    ->Args({kDefault})
    ->Args({kBounds})
    ->Args({kBoundsAndRtree})
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_DisplayListBuiderWithSaveLayerAndImageFilter)
    ->Args({kDefault})
    ->Args({kBounds})
    ->Args({kBoundsAndRtree})
    ->Unit(benchmark::kMillisecond);

}  // namespace flutter
