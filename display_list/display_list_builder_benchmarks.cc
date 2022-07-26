// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/benchmarking/benchmarking.h"
#include "flutter/display_list/display_list_test_utils.h"

namespace flutter {

static void BM_DisplayListBuiderBuild(benchmark::State& state) {
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    for (auto& group : testing::allGroups) {
      for (size_t i = 0; i < group.variants.size(); i++) {
        auto& invocation = group.variants[i];
        invocation.Invoke(builder);
      }
    }
    auto display_list = builder.Build();
  }
}

static void BM_DisplayListBuiderBuildAndGetBounds(benchmark::State& state) {
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    for (auto& group : testing::allGroups) {
      for (size_t i = 0; i < group.variants.size(); i++) {
        auto& invocation = group.variants[i];
        invocation.Invoke(builder);
      }
    }
    auto display_list = builder.Build();
    display_list->bounds();
  }
}

static void BM_DisplayListBuiderBuildAndGetBoundsAndRtree(
    benchmark::State& state) {
  while (state.KeepRunning()) {
    DisplayListBuilder builder;
    for (auto& group : testing::allGroups) {
      for (size_t i = 0; i < group.variants.size(); i++) {
        auto& invocation = group.variants[i];
        invocation.Invoke(builder);
      }
    }
    auto display_list = builder.Build();
    display_list->bounds();
    display_list->rtree();
  }
}

BENCHMARK(BM_DisplayListBuiderBuild)->Unit(benchmark::kMillisecond);

BENCHMARK(BM_DisplayListBuiderBuildAndGetBounds)->Unit(benchmark::kMillisecond);

BENCHMARK(BM_DisplayListBuiderBuildAndGetBoundsAndRtree)
    ->Unit(benchmark::kMillisecond);

}  // namespace flutter
