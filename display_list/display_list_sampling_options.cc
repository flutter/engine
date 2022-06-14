// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_sampling_options.h"

namespace flutter {
DlSamplingOptions::DlSamplingOptions(const SkSamplingOptions& skSamplingOptions)
    : maxAniso(skSamplingOptions.maxAniso),
      useCubic(skSamplingOptions.useCubic),
      cubic({skSamplingOptions.cubic.B, skSamplingOptions.cubic.C}),
      filter(ToDl(skSamplingOptions.filter)),
      mipmap(ToDl(skSamplingOptions.mipmap)) {}
}  // namespace flutter
