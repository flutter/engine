// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_GPU_GPU_STUDIO_SOFTWARE_H_
#define FLUTTER_SHELL_GPU_GPU_STUDIO_SOFTWARE_H_

#include "flutter/flow/studio.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/gpu/gpu_surface_software_delegate.h"

namespace flutter {

class GPUStudioSoftware : public Studio {
 public:
  GPUStudioSoftware(GPUSurfaceSoftwareDelegate* delegate);

  ~GPUStudioSoftware() override;

  // |Studio|
  bool IsValid() override;

  // |Studio|
  GrDirectContext* GetContext() override;

 private:
  GPUSurfaceSoftwareDelegate* delegate_;

  FML_DISALLOW_COPY_AND_ASSIGN(GPUStudioSoftware);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_STUDIO_SOFTWARE_H_
