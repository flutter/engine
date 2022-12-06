// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_D3D12_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_D3D12_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_d3d12.h"
#include "flutter/shell/platform/embedder/embedder_external_view_embedder.h"
#include "flutter/shell/platform/embedder/embedder_surface.h"
#include "shell/common/context_options.h"
#include "shell/platform/embedder/embedder.h"
#include "include/gpu/d3d/GrD3DTypes.h"

namespace flutter {

class EmbedderSurfaceD3D12 final : public EmbedderSurface,
                                    public GPUSurfaceD3D12Delegate {
 public:
  struct D3D12SwapchainDispatchTable {
    std::function<FlutterD3D12Resource(const SkISize& frame_size)>
        acquire_back_buffer;  // required
    std::function<bool()>
        present_back_buffer;  // required
  };

  EmbedderSurfaceD3D12(
      IDXGIAdapter1* dxgi_adapter,
      ID3D12Device* device,
      ID3D12CommandQueue* queue,
      D3D12SwapchainDispatchTable swapchain_dispatch_table,
      std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder);

  ~EmbedderSurfaceD3D12() override;

  // |GPUSurfaceD3D12Delegate|
  ID3D12Resource* AcquireBackBuffer(const SkISize& size) override;

  // |GPUSurfaceD3D12Delegate|
  bool PresentBackBuffer() override;

 private:
  bool valid_ = false;
  gr_cp<IDXGIAdapter1> dxgi_adapter_;
  gr_cp<ID3D12Device> device_;
  gr_cp<ID3D12CommandQueue> queue_;
  D3D12SwapchainDispatchTable swapchain_dispatch_table_;
  std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder_;
  sk_sp<GrDirectContext> main_context_;
  sk_sp<GrDirectContext> resource_context_;

  // |EmbedderSurface|
  bool IsValid() const override;

  // |EmbedderSurface|
  std::unique_ptr<Surface> CreateGPUSurface() override;

  // |EmbedderSurface|
  sk_sp<GrDirectContext> CreateResourceContext() const override;

  sk_sp<GrDirectContext> CreateGrContext(ContextType context_type) const;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderSurfaceD3D12);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_SURFACE_D3D12_H_
