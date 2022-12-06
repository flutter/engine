// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_d3d12.h"

#include "flutter/shell/common/shell_io_manager.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"

namespace flutter {

EmbedderSurfaceD3D12::EmbedderSurfaceD3D12(
    IDXGIAdapter1* dxgi_adapter,
    ID3D12Device* device,
    ID3D12CommandQueue* queue,
    D3D12SwapchainDispatchTable swapchain_dispatch_table,
    std::shared_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : swapchain_dispatch_table_(swapchain_dispatch_table),
      external_view_embedder_(external_view_embedder) {
  dxgi_adapter_.retain(dxgi_adapter);
  device_.retain(device);
  queue_.retain(queue);

  // Make sure all required members of the dispatch table are checked.
  if (!swapchain_dispatch_table_.acquire_back_buffer ||
      !swapchain_dispatch_table_.present_back_buffer) {
    return;
  }

  main_context_ = CreateGrContext(ContextType::kRender);
  // TODO(96954): Add a second (optional) queue+family index to the Embedder API
  //              to allow embedders to specify a dedicated transfer queue for
  //              use by the resource context. Queue families with graphics
  //              capability can always be used for memory transferring, but it
  //              would be advantageous to use a dedicated transter queue here.
  resource_context_ = CreateGrContext(ContextType::kResource);

  valid_ = main_context_ && resource_context_;
}

EmbedderSurfaceD3D12::~EmbedderSurfaceD3D12() {
  if (main_context_) {
    main_context_->releaseResourcesAndAbandonContext();
  }
  if (resource_context_) {
    resource_context_->releaseResourcesAndAbandonContext();
  }
}

// |GPUSurfaceD3D12Delegate|
ID3D12Resource* EmbedderSurfaceD3D12::AcquireBackBuffer(const SkISize& size) {
  return static_cast<ID3D12Resource*>(swapchain_dispatch_table_.acquire_back_buffer(size));
}

// |GPUSurfaceD3D12Delegate|
bool EmbedderSurfaceD3D12::PresentBackBuffer() {
  return swapchain_dispatch_table_.present_back_buffer();
}

// |EmbedderSurface|
bool EmbedderSurfaceD3D12::IsValid() const {
  return valid_;
}

// |EmbedderSurface|
std::unique_ptr<Surface> EmbedderSurfaceD3D12::CreateGPUSurface() {
  return std::make_unique<GPUSurfaceD3D12>(this, main_context_);
}

// |EmbedderSurface|
sk_sp<GrDirectContext> EmbedderSurfaceD3D12::CreateResourceContext() const {
  return resource_context_;
}

sk_sp<GrDirectContext> EmbedderSurfaceD3D12::CreateGrContext(ContextType context_type) const {
  GrD3DBackendContext backend_context = {};
  backend_context.fAdapter = dxgi_adapter_;
  backend_context.fDevice = device_;
  backend_context.fQueue = queue_;

  GrContextOptions options =
      MakeDefaultContextOptions(context_type, GrBackendApi::kDirect3D);
  options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
  return GrDirectContext::MakeDirect3D(backend_context, options);
}

}  // namespace flutter
