// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_SURFACE_SURFACE_VULKAN_IMPELLER_H_
#define FLUTTER_SHELL_SURFACE_SURFACE_VULKAN_IMPELLER_H_

#include "flutter/common/graphics/gl_context_switch.h"
#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/impeller/aiks/aiks_context.h"
#include "flutter/impeller/renderer/context.h"
#include "flutter/shell/surface/surface_vulkan_delegate.h"

namespace flutter {

class SurfaceVulkanImpeller final : public Surface {
 public:
  explicit SurfaceVulkanImpeller(std::shared_ptr<impeller::Context> context);

  // |Surface|
  ~SurfaceVulkanImpeller() override;

  // |Surface|
  bool IsValid() override;

 private:
  std::shared_ptr<impeller::Context> impeller_context_;
  std::shared_ptr<impeller::AiksContext> aiks_context_;
  bool is_valid_ = false;

  // |Surface|
  std::unique_ptr<SurfaceFrame> AcquireFrame(const SkISize& size) override;

  // |Surface|
  SkMatrix GetRootTransformation() const override;

  // |Surface|
  GrDirectContext* GetContext() override;

  // |Surface|
  std::unique_ptr<GLContextResult> MakeRenderContextCurrent() override;

  // |Surface|
  bool EnableRasterCache() const override;

  // |Surface|
  std::shared_ptr<impeller::AiksContext> GetAiksContext() const override;

  FML_DISALLOW_COPY_AND_ASSIGN(SurfaceVulkanImpeller);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_SURFACE_SURFACE_VULKAN_IMPELLER_H_
