// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/benchmarking/benchmarking.h"

#include "flutter/display_list/geometry/dl_region.h"
#include "third_party/skia/include/core/SkRegion.h"

#include <random>

namespace {

class SkRegionAdapter {
 public:
  void addRects(std::vector<SkIRect>&& rects) {
    for (const auto& rect : rects) {
      region_.op(rect, SkRegion::kUnion_Op);
    }
  }

  SkIRect getBounds() { return region_.getBounds(); }

  void addRegion(const SkRegionAdapter& region) {
    region_.op(region.region_, SkRegion::kUnion_Op);
  }

  bool intersects(const SkRegionAdapter& region) {
    return region_.intersects(region.region_);
  }

  bool intersects(const SkIRect& rect) { return region_.intersects(rect); }

  std::vector<SkIRect> getRects() {
    std::vector<SkIRect> rects;
    SkRegion::Iterator it(region_);
    while (!it.done()) {
      rects.push_back(it.rect());
      it.next();
    }
    return rects;
  }

 private:
  SkRegion region_;
};

class DlRegionAdapter {
 public:
  void addRects(std::vector<SkIRect>&& rects) {
    region_.addRects(std::move(rects));
  }

  void addRegion(const DlRegionAdapter& region) {
    region_.addRegion(region.region_);
  }

  SkIRect getBounds() { return region_.bounds(); }

  bool intersects(const DlRegionAdapter& region) {
    return region_.intersects(region.region_);
  }

  bool intersects(const SkIRect& rect) { return region_.intersects(rect); }

  std::vector<SkIRect> getRects() { return region_.getRects(false); }

  DlRegionAdapter() {}

  DlRegionAdapter(const DlRegionAdapter& copy) : region_(copy.region_, true) {}

 private:
  flutter::DlRegion region_;
};

template <typename Region>
void RunAddRectsBenchmark(benchmark::State& state, int maxSize) {
  std::random_device d;
  std::seed_seq seed{2, 1, 3};
  std::mt19937 rng(seed);

  std::uniform_int_distribution pos(0, 4000);
  std::uniform_int_distribution size(1, maxSize);

  std::vector<SkIRect> rects;
  for (int i = 0; i < 2000; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
    rects.push_back(rect);
  }

  while (state.KeepRunning()) {
    Region region;
    region.addRects(std::move(rects));
    auto vec2 = region.getRects();
  }
}

template <typename Region>
void RunAddRegionBenchmark(benchmark::State& state, int maxSize) {
  std::random_device d;
  std::seed_seq seed{2, 1, 3};
  std::mt19937 rng(seed);

  std::uniform_int_distribution pos(0, 4000);
  std::uniform_int_distribution size(1, maxSize);

  std::vector<SkIRect> rects;
  for (int i = 0; i < 500; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
    rects.push_back(rect);
  }

  Region base;

  Region region1(base);
  region1.addRects(std::move(rects));

  rects.clear();
  for (int i = 0; i < 500; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
    rects.push_back(rect);
  }
  Region region2(base);
  region2.addRects(std::move(rects));

  while (state.KeepRunning()) {
    Region copy_of_region1(region1);
    copy_of_region1.addRegion(region2);
    // Region copy_of_region2(region2);
    // copy_of_region2.addRegion(region1);
  }
}

template <typename Region>
void RunIntersectsRegionBenchmark(benchmark::State& state, int maxSize) {
  std::random_device d;
  std::seed_seq seed{2, 1, 3};
  std::mt19937 rng(seed);

  std::uniform_int_distribution pos(0, 4000);
  std::uniform_int_distribution size(1, maxSize);

  std::vector<SkIRect> rects;
  for (int i = 0; i < 500; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
    rects.push_back(rect);
  }

  Region base;

  Region region1(base);
  region1.addRects(std::move(rects));

  rects.clear();
  for (int i = 0; i < 500; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
    rects.push_back(rect);
  }
  Region region2(base);
  region2.addRects(std::move(rects));

  while (state.KeepRunning()) {
    region1.intersects(region2);
  }
}

template <typename Region>
void RunIntersectsSingleRectBenchmark(benchmark::State& state, int maxSize) {
  std::random_device d;
  std::seed_seq seed{2, 1, 3};
  std::mt19937 rng(seed);

  std::uniform_int_distribution pos(0, 4000);
  std::uniform_int_distribution size(1, maxSize);

  std::vector<SkIRect> rects;
  for (int i = 0; i < 500; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
    rects.push_back(rect);
  }

  Region base;

  Region region1(base);
  region1.addRects(std::move(rects));

  rects.clear();
  for (int i = 0; i < 100; ++i) {
    SkIRect rect = SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
    rects.push_back(rect);
  }

  while (state.KeepRunning()) {
    for (auto& rect : rects) {
      region1.intersects(rect);
    }
  }
}

}  // namespace

namespace flutter {

static void BM_DlRegion_AddRects(benchmark::State& state, int maxSize) {
  RunAddRectsBenchmark<DlRegionAdapter>(state, maxSize);
}

static void BM_SkRegion_AddRects(benchmark::State& state, int maxSize) {
  RunAddRectsBenchmark<SkRegionAdapter>(state, maxSize);
}

static void BM_DlRegion_AddRegion(benchmark::State& state, int maxSize) {
  RunAddRegionBenchmark<DlRegionAdapter>(state, maxSize);
}

static void BM_SkRegion_AddRegion(benchmark::State& state, int maxSize) {
  RunAddRegionBenchmark<SkRegionAdapter>(state, maxSize);
}

static void BM_DlRegion_IntersectsRegion(benchmark::State& state, int maxSize) {
  RunIntersectsRegionBenchmark<DlRegionAdapter>(state, maxSize);
}

static void BM_SkRegion_IntersectsRegion(benchmark::State& state, int maxSize) {
  RunIntersectsRegionBenchmark<SkRegionAdapter>(state, maxSize);
}

static void BM_DlRegion_IntersectsSingleRect(benchmark::State& state,
                                             int maxSize) {
  RunIntersectsSingleRectBenchmark<DlRegionAdapter>(state, maxSize);
}

static void BM_SkRegion_IntersectsSingleRect(benchmark::State& state,
                                             int maxSize) {
  RunIntersectsSingleRectBenchmark<SkRegionAdapter>(state, maxSize);
}

BENCHMARK_CAPTURE(BM_DlRegion_IntersectsSingleRect, Tiny, 30)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsSingleRect, Tiny, 30)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_DlRegion_IntersectsSingleRect, Small, 100)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsSingleRect, Small, 100)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_DlRegion_IntersectsSingleRect, Medium, 400)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsSingleRect, Medium, 400)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_DlRegion_IntersectsSingleRect, Large, 1500)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsSingleRect, Large, 1500)
    ->Unit(benchmark::kNanosecond);

BENCHMARK_CAPTURE(BM_DlRegion_IntersectsRegion, Tiny, 30)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsRegion, Tiny, 30)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_DlRegion_IntersectsRegion, Small, 100)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsRegion, Small, 100)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_DlRegion_IntersectsRegion, Medium, 400)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsRegion, Medium, 400)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_DlRegion_IntersectsRegion, Large, 1500)
    ->Unit(benchmark::kNanosecond);
BENCHMARK_CAPTURE(BM_SkRegion_IntersectsRegion, Large, 1500)
    ->Unit(benchmark::kNanosecond);

BENCHMARK_CAPTURE(BM_DlRegion_AddRegion, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRegion, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_DlRegion_AddRegion, Small, 100)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRegion, Small, 100)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_DlRegion_AddRegion, Medium, 400)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRegion, Medium, 400)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_DlRegion_AddRegion, Large, 1500)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRegion, Large, 1500)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_CAPTURE(BM_DlRegion_AddRects, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRects, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_DlRegion_AddRects, Small, 100)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRects, Small, 100)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_DlRegion_AddRects, Medium, 400)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRects, Medium, 400)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_DlRegion_AddRects, Large, 1500)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_SkRegion_AddRects, Large, 1500)
    ->Unit(benchmark::kMicrosecond);

}  // namespace flutter