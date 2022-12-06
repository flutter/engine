// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_GPU_GPU_SURFACE_D3D12_DELEGATE_H_
#define FLUTTER_SHELL_GPU_GPU_SURFACE_D3D12_DELEGATE_H_

#include "flutter/fml/memory/ref_ptr.h"
#include "third_party/skia/include/core/SkSize.h"

#include <d3d12.h>
#include <dxgi1_4.h>

namespace flutter {

// struct GpuSurfaceD3D12Context {
//   IDXGIAdapter1* adapter;
//   ID3D12Device* device;
//   ID3D12CommandQueue* queue;
// };

//------------------------------------------------------------------------------
/// @brief      Interface implemented by all platform surfaces that can present
///             a D3D12 backing store to the "screen". The GPU surface
///             abstraction (which abstracts the client rendering API) uses this
///             delegation pattern to tell the platform surface (which abstracts
///             how backing stores fulfilled by the selected client rendering
///             API end up on the "screen" on a particular platform) when the
///             rasterizer needs to allocate and present the D3D12 backing
///             store.
///
/// @see        |EmbedderSurfaceD3D12|.
///
class GPUSurfaceD3D12Delegate {
 public:
  virtual ~GPUSurfaceD3D12Delegate();

  // virtual const GpuSurfaceD3D12Context& Context() = 0;

  /// @brief  Called by the engine to fetch an ID3D12Resource for writing the next
  ///         frame.
  ///
  virtual ID3D12Resource* AcquireBackBuffer(const SkISize& size) = 0;

  /// @brief  Called by the engine once a frame has been rendered to the back buffer
  ///         and it's ready to be presented.
  ///
  virtual bool PresentBackBuffer() = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_SURFACE_D3D12_DELEGATE_H_
