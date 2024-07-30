// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_SURFACE_SURFACE_SOFTWARE_H_
#define FLUTTER_SHELL_SURFACE_SURFACE_SOFTWARE_H_

#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/surface/surface_software_delegate.h"

namespace flutter {

class SurfaceSoftware : public Surface {
 public:
  SurfaceSoftware(SurfaceSoftwareDelegate* delegate, bool render_to_surface);

  ~SurfaceSoftware() override;

  // |Surface|
  bool IsValid() override;

  // |Surface|
  std::unique_ptr<SurfaceFrame> AcquireFrame(const SkISize& size) override;

  // |Surface|
  SkMatrix GetRootTransformation() const override;

  // |Surface|
  GrDirectContext* GetContext() override;

 private:
  SurfaceSoftwareDelegate* delegate_;
  // TODO(38466): Refactor GPU surface APIs take into account the fact that an
  // external view embedder may want to render to the root surface. This is a
  // hack to make avoid allocating resources for the root surface when an
  // external view embedder is present.
  const bool render_to_surface_;
  fml::TaskRunnerAffineWeakPtrFactory<SurfaceSoftware> weak_factory_;
  FML_DISALLOW_COPY_AND_ASSIGN(SurfaceSoftware);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_SURFACE_SURFACE_SOFTWARE_H_
