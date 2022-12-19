// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_D3D12_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_D3D12_H_

#include <stdint.h>
#include "flutter/shell/platform/windows/angle_surface_manager.h"
#include "flutter/shell/platform/windows/window_surface.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include <d3d11_4.h>
#include <dcomp.h>
#include <forward_list>
#include <memory>

namespace flutter {

struct DCompVisual;

// Represents a rendering surface for a platform window.
class WindowSurfaceD3D12 : public WindowSurface {
public:
  WindowSurfaceD3D12();
  virtual ~WindowSurfaceD3D12();

  // Creates a surface using D3D12 that can be bound to a window.
  // Returns nullptr if D3D12 not supported.
  static std::unique_ptr<WindowSurfaceD3D12> Create();

  // |WindowSurface|
  void Init(PlatformWindow window, unsigned int width, unsigned int height) override;

  // |WindowSurface|
  void Destroy() override;

  // |WindowSurface|
  void GetSurfaceDimensions(unsigned int &width, unsigned int &height) override;

  // |WindowSurface|
  void OnResize(unsigned int width, unsigned int height) override;

  // |WindowSurface|
  void InitRendererConfig(FlutterRendererConfig& config) override;

  // |WindowSurface|
  void CleanUpRendererConfig(FlutterRendererConfig& config) override;

  // |WindowSurface|
  bool InitCompositorConfig(FlutterCompositor& config) override;

  // |WindowSurface|
  void* GetCompositionDevice() override;

private:
  std::unique_ptr<DCompVisual> CreateVisual(unsigned int width, unsigned int height);

  PlatformWindow window_;
  gr_cp<IDCompositionTarget> dcomp_target_;
  gr_cp<IUnknown> surface_;

  gr_cp<IDXGIFactory4> dxgi_factory_;
  gr_cp<IDXGIAdapter1> dxgi_adapter_;
  gr_cp<ID3D12Device> device_;
  gr_cp<ID3D12CommandQueue> queue_;

  gr_cp<ID3D11Device5> device11_;
  gr_cp<IDCompositionDevice> dcomp_device_;

  HANDLE fence_event_;
  gr_cp<ID3D12Fence> fence_;
  gr_cp<ID3D11Fence> fence11_;
  uint64_t fence_value_ = 0;

  std::unique_ptr<DCompVisual> root_visual_;
  std::unique_ptr<DCompVisual> visuals_cache_;
  // NOTE: Tail is only used for insertion
  DCompVisual* visuals_cache_tail_ = nullptr;
  std::forward_list<IDCompositionVisual*> shown_visuals_;
  HDWP deferred_window_positions_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINDOW_SURFACE_D3D12_H_

