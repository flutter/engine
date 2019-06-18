// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "benchmark/benchmark_api.h"

namespace fml {
namespace benchmarking {

static void BM_StringCreation(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::string empty_string;
  }
}

BENCHMARK(BM_StringCreation);

int Main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}

}  // namespace benchmarking
}  // namespace fml

int main(int argc, char** argv) {
  return fml::benchmarking::Main(argc, argv);
}
