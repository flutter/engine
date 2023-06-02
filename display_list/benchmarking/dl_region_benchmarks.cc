// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/benchmarking/benchmarking.h"

#include "flutter/display_list/geometry/dl_region.h"
#include "third_party/skia/include/core/SkRegion.h"

#include <random>

class SkRegionAdapter {
 public:
  void addRect(const SkIRect& rect) { region_.op(rect, SkRegion::kUnion_Op); }

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

class DlRegion2Adapter {
 public:
  void addRect(const SkIRect& rect) { rects_.push_back(rect); }

  std::vector<SkIRect> getRects() {
    flutter::DlRegion2 region;
    region.addRects(std::move(rects_));
    return region.getRects(false);
  }

 private:
  std::vector<SkIRect> rects_;
};

class DlRegionSortedAdapter {
 public:
  void addRect(const SkIRect& rect) { rects_.push_back(rect); }

  std::vector<SkIRect> getRects() {
    flutter::DlRegionSorted region;
    region.addRects(std::move(rects_));
    return region.getRects(false);
  }

 private:
  std::vector<SkIRect> rects_;
};

template <typename Region>
void RunRegionBenchmark(benchmark::State& state, int maxSize) {
  while (state.KeepRunning()) {
    std::random_device d;
    std::seed_seq seed{2, 1, 3};
    std::mt19937 rng(seed);

    std::uniform_int_distribution pos(0, 4000);
    std::uniform_int_distribution size(1, maxSize);

    Region region;

    for (int i = 0; i < 2000; ++i) {
      SkIRect rect =
          SkIRect::MakeXYWH(pos(rng), pos(rng), size(rng), size(rng));
      region.addRect(rect);
    }

    auto vec2 = region.getRects();
  }
}

namespace flutter {

static void BM_RegionBenchmarkSkRegion(benchmark::State& state, int maxSize) {
  RunRegionBenchmark<SkRegionAdapter>(state, maxSize);
}

static void BM_RegionBenchmarkDlRegion(benchmark::State& state, int maxSize) {
  RunRegionBenchmark<DlRegion>(state, maxSize);
}

static void BM_RegionBenchmarkDlRegionSorted(benchmark::State& state,
                                             int maxSize) {
  RunRegionBenchmark<DlRegionSortedAdapter>(state, maxSize);
}

static void BM_RegionBenchmarkDlRegion2(benchmark::State& state, int maxSize) {
  RunRegionBenchmark<DlRegion2Adapter>(state, maxSize);
}

BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion2, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegionSorted, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkSkRegion, Tiny, 30)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion, Small, 100)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion2, Small, 100)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegionSorted, Small, 100)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkSkRegion, Small, 100)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion, Medium, 400)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion2, Medium, 400)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegionSorted, Medium, 400)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkSkRegion, Medium, 400)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion, Large, 1500)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegion2, Large, 1500)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkDlRegionSorted, Large, 1500)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_RegionBenchmarkSkRegion, Large, 1500)
    ->Unit(benchmark::kMicrosecond);

}  // namespace flutter