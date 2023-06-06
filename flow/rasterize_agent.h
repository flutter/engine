// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_FLOW_RASTERIZE_AGENT_H_
#define SHELL_FLOW_RASTERIZE_AGENT_H_

#include "flutter/common/graphics/gl_context_switch.h"
#include "flutter/display_list/dl_canvas.h"
#include "flutter/flow/embedded_views.h"
#include "flutter/flow/surface_frame.h"
#include "flutter/fml/raster_thread_merger.h"

namespace flutter {

class RasterizeAgent {
 public:
  RasterizeAgent() = default;
  virtual ~RasterizeAgent() = default;
  virtual void PreTearDown() = 0;
  virtual bool SupportsDynamicThreadMerging() = 0;
  virtual bool AllowsPartialRepaint() = 0;
  virtual DlCanvas* OnBeginFrame(
      const SkISize& frame_size,
      float pixel_ratio,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) = 0;
  virtual void OnEndFrame(
      bool should_resubmit_frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) = 0;
  virtual void SubmitFrame(
      std::unique_ptr<SurfaceFrame> frame,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) = 0;
  virtual PostPrerollResult PostPrerollAction(
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) = 0;

  virtual ExternalViewEmbedder* ViewEmbedder() = 0;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(RasterizeAgent);
};

}  // namespace flutter

#endif  // SHELL_FLOW_RASTERIZE_AGENT_H_
