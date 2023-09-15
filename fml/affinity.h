// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace fml {

/// The CPU Affinity provides a hint to the operating system on which cores a
/// particular thread should be scheduled on. The operating system may or may
/// not honor these requests.
enum class CpuAffinity {
  /// @brief Request CPU affinity for the performance cores.
  ///
  ///        Generally speaking, only the UI and Raster thread should
  ///        use this option.
  kPerformance,

  /// @brief Request CPU affinity for the efficiency cores.
  kEfficiency,
};

/// @brief Request that the current thread switch to the given `affinity`.
bool RequestAffinity(CpuAffinity affinity);

}
