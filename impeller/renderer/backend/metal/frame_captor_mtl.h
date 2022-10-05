// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <Metal/Metal.h>

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/frame_captor.h"

namespace impeller {

class FrameCaptorMTL final : public FrameCaptor,
                             public BackendCast<FrameCaptorMTL, FrameCaptor> {
 public:
  // |FrameCaptor|
  ~FrameCaptorMTL() override;

  bool StartCapturingFrame(FrameCaptorConfiguration configuration) override;

  bool StopCapturingFrame() override;

 private:
  friend class ContextMTL;

  id<MTLDevice> device_;
  FrameCaptorMTL(id<MTLDevice> device);

  FML_DISALLOW_COPY_AND_ASSIGN(FrameCaptorMTL);
};

}  // namespace impeller
