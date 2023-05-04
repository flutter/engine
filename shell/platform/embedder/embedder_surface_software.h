// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_SOFTWARE_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_SOFTWARE_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_software.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_studio_software.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"

#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {

class EmbedderSurfaceSoftware final : public EmbedderSurface {
 public:
  EmbedderSurfaceSoftware(EmbedderStudioSoftware* studio,
                          bool render_to_surface);

  ~EmbedderSurfaceSoftware() override;

  // |EmbedderSurface|
  bool IsValid() const override;

 private:
  EmbedderStudioSoftware* studio_;
  bool render_to_surface_;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceSoftware);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_SOFTWARE_H_
