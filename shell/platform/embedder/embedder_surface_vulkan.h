// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_VULKAN_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_VULKAN_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/common/context_options.h"
#include "flutter/shell/gpu/gpu_surface_vulkan.h"
#include "flutter/shell/gpu/gpu_surface_vulkan_delegate.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_studio_vulkan.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"
#include "flutter/vulkan/procs/vulkan_proc_table.h"

namespace flutter {

class EmbedderSurfaceVulkan final : public EmbedderSurface {
 public:
  EmbedderSurfaceVulkan(EmbedderStudioVulkan* studio,
                        sk_sp<GrDirectContext> main_context,
                        bool render_to_surface);

  ~EmbedderSurfaceVulkan() override;

  // |EmbedderSurface|
  bool IsValid() const override;

 private:
  sk_sp<GrDirectContext> main_context_;
  EmbedderStudioVulkan* studio_;
  bool render_to_surface_;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  // |EmbedderSurface|
  sk_sp<GrDirectContext> CreateResourceContext() const override;

  sk_sp<GrDirectContext> CreateGrContext(VkInstance instance,
                                         uint32_t version,
                                         size_t instance_extension_count,
                                         const char** instance_extensions,
                                         size_t device_extension_count,
                                         const char** device_extensions,
                                         ContextType context_type) const;

  void* GetInstanceProcAddress(VkInstance instance, const char* proc_name);

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceVulkan);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_VULKAN_H_
