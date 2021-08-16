// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DEFAULT_FLATLAND_CONNECTION_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DEFAULT_FLATLAND_CONNECTION_H_

#include <fuchsia/scenic/scheduling/cpp/fidl.h>
#include <fuchsia/ui/composition/cpp/fidl.h>

#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/time/time_delta.h"

#include <string>

namespace flutter_runner {

using on_frame_presented_event =
    std::function<void(fuchsia::scenic::scheduling::FramePresentedInfo)>;

// The component residing on the raster thread that is responsible for
// maintaining the Flatland instance connection and presenting updates.
class FlatlandConnection final {
 public:
  FlatlandConnection(std::string debug_label,
                     fml::closure error_callback,
                     on_frame_presented_event on_frame_presented_callback,
                     uint64_t max_frames_in_flight,
                     fml::TimeDelta vsync_offset);

  ~FlatlandConnection();

  FML_DISALLOW_COPY_AND_ASSIGN(FlatlandConnection);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DEFAULT_FLATLAND_CONNECTION_H_
