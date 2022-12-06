// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_SURFACE_D3D12_H_
#define SHELL_GPU_GPU_SURFACE_D3D12_H_

#include <memory>

#include "flutter/flow/surface.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/gpu/gpu_surface_d3d12_delegate.h"
#include "include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkSize.h"

#include <dxgi1_4.h>

namespace flutter {

//------------------------------------------------------------------------------
/// @brief  A GPU surface backed by ID3D12Resources provided by a
///         GPUSurfaceD3D12Delegate.
///
class GPUSurfaceD3D12 : public Surface {
 public:
  //------------------------------------------------------------------------------
  /// @brief      Create a GPUSurfaceD3D12 while letting it reuse an existing
  ///             GrDirectContext.
  ///
  GPUSurfaceD3D12(GPUSurfaceD3D12Delegate* delegate,
                  const sk_sp<GrDirectContext>& context);

  ~GPUSurfaceD3D12() override;

  // |Surface|
  bool IsValid() override;

  // |Surface|
  std::unique_ptr<SurfaceFrame> AcquireFrame(const SkISize& size) override;

  // |Surface|
  SkMatrix GetRootTransformation() const override;

  // |Surface|
  GrDirectContext* GetContext() override;

  static SkColorType ColorTypeFromFormat(const DXGI_FORMAT format);

 private:
  GPUSurfaceD3D12Delegate* delegate_;
  sk_sp<GrDirectContext> skia_context_;
  SkISize last_frame_size_;

  fml::WeakPtrFactory<GPUSurfaceD3D12> weak_factory_;

  sk_sp<SkSurface> CreateSurfaceFromResource(ID3D12Resource* image,
                                                const DXGI_FORMAT format,
                                                const SkISize& size);

  FML_DISALLOW_COPY_AND_ASSIGN(GPUSurfaceD3D12);
};

}  // namespace flutter

#endif  // SHELL_GPU_GPU_SURFACE_D3D12_H_
