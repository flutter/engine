#include "flutter/shell/platform/windows/window_surface_d3d12.h"

#include "flutter/fml/logging.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "flutter/shell/platform/windows/platform_views.h"

#include <Shlwapi.h>
#include <combaseapi.h>
#include <d3d11_4.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcommon.h>
#include <dcommon.h>
#include <dcomp.h>
#include <dwmapi.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxgiformat.h>
#include <d2d1helper.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <unknwnbase.h>
#include <winbase.h>
#include <winerror.h>
#include <winuser.h>
#include <cstdint>
#include <memory>

namespace flutter {

#define GR_D3D_CALL_ERRCHECK(X)                                    \
  do {                                                             \
    HRESULT result = X;                                            \
    if (FAILED(result)) {                                          \
      FML_LOG(ERROR) << "Failed Direct3D call. Error: " << result; \
    }                                                              \
    SkASSERT(SUCCEEDED(result));                                   \
  } while (false)

// TODO: Move compositor out of this file. We should be able to composite with ANGLE as well.

struct VisualConfig {
  unsigned int width, height;
  DXGI_FORMAT format;

  VisualConfig(unsigned int width, unsigned int height) : width(width), height(height), format(DXGI_FORMAT_R8G8B8A8_UNORM) {}
};

struct SwapChainStrategy {
  gr_cp<IDXGISwapChain3> swap_chain_;
  unsigned int back_buffer_index_ = 999;

  ID3D12Resource* Acquire(const VisualConfig& config) {
    back_buffer_index_ = swap_chain_->GetCurrentBackBufferIndex();
    ID3D12Resource* back_buffer;
    GR_D3D_CALL_ERRCHECK(
        swap_chain_->GetBuffer(back_buffer_index_, IID_PPV_ARGS(&back_buffer)));
    return back_buffer;
  }

  void Present(const VisualConfig& config) {
    GR_D3D_CALL_ERRCHECK(swap_chain_->Present(0, 0));
  }

  void Resize(IDCompositionVisual* visual, const VisualConfig& config) {
    GR_D3D_CALL_ERRCHECK(
        swap_chain_->ResizeBuffers(0, config.width, config.height, config.format, 0));
  }
};

struct SurfaceStrategy {
  gr_cp<IDCompositionSurface> dcomp_surface_;

  ID3D12Device* device_;
  ID3D11Device* device11_;
  IDCompositionDevice* dcomp_device_;

  gr_cp<ID3D11Texture2D> offscreen_texture_11_;
  gr_cp<ID3D12Resource> offscreen_texture_;

  SurfaceStrategy(ID3D12Device* device, ID3D11Device* device11, IDCompositionDevice* dcomp_device) :
  device_(device), device11_(device11), dcomp_device_(dcomp_device) {}

  static bool SupportsFormat(DXGI_FORMAT format) {
    return format == DXGI_FORMAT_R8G8B8A8_UNORM || format == DXGI_FORMAT_R16G16B16A16_FLOAT;
  }

  ID3D12Resource* Acquire(const VisualConfig& config) {
    if (!offscreen_texture_11_) {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = config.width;
      desc.Height = config.height;
      desc.MipLevels = 1;
      desc.ArraySize = 1;
      desc.Format = config.format;
      desc.SampleDesc.Count = 1;
      desc.BindFlags = D3D11_BIND_RENDER_TARGET;
      desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

      GR_D3D_CALL_ERRCHECK(device11_->CreateTexture2D(&desc, nullptr, &offscreen_texture_11_));

      HANDLE handle;

      gr_cp<IDXGIResource> dxgi;
      GR_D3D_CALL_ERRCHECK(offscreen_texture_11_->QueryInterface(IID_PPV_ARGS(&dxgi)));
      GR_D3D_CALL_ERRCHECK(dxgi->GetSharedHandle(&handle));

      GR_D3D_CALL_ERRCHECK(device_->OpenSharedHandle(handle, IID_PPV_ARGS(&offscreen_texture_)));
    }

    auto resource = offscreen_texture_.get();
    resource->AddRef();
    return resource;
  }

  void Present(const VisualConfig& config) {
    gr_cp<IDXGIResource1> draw_resource;
    POINT update_offset;
    GR_D3D_CALL_ERRCHECK(dcomp_surface_->BeginDraw(nullptr, IID_PPV_ARGS(&draw_resource), &update_offset));
    gr_cp<ID3D11Texture2D> draw_texture;
    GR_D3D_CALL_ERRCHECK(draw_resource->QueryInterface(IID_PPV_ARGS(&draw_texture)));

    // Copy offscreen texture into DirectComposition surface
    // Assumes a fence has been used between render and present.
    D3D11_BOX box = {};
    box.left = 0;
    box.top = 0;
    box.right = box.left + config.width;
    box.bottom = box.top + config.height;
    box.front = 0;
    box.back = 1;

    gr_cp<ID3D11DeviceContext> context;
    device11_->GetImmediateContext(&context);
    context->CopySubresourceRegion(draw_texture.get(), 0, update_offset.x,
                                    update_offset.y, 0,
                                    offscreen_texture_11_.get(), 0, &box);

    GR_D3D_CALL_ERRCHECK(dcomp_surface_->EndDraw());
  }

  void Resize(IDCompositionVisual* visual, const VisualConfig& config) {
    dcomp_surface_.reset();
    offscreen_texture_11_.reset();
    offscreen_texture_.reset();

    GR_D3D_CALL_ERRCHECK(dcomp_device_->CreateSurface(config.width, config.height, config.format, DXGI_ALPHA_MODE_PREMULTIPLIED, &dcomp_surface_));

    GR_D3D_CALL_ERRCHECK(visual->SetContent(dcomp_surface_.get()));
  }
};

using PresentStrategy = std::variant<SwapChainStrategy, SurfaceStrategy>;

struct DCompVisual {
  std::unique_ptr<DCompVisual> next_;
  gr_cp<IDCompositionVisual> dcomp_visual_;
  gr_cp<ID3D12Resource> acquired_image_;
  VisualConfig config_;
  PresentStrategy strategy_;

  DCompVisual(gr_cp<IDCompositionVisual>&& visual, PresentStrategy&& strategy, VisualConfig&& config) :
  dcomp_visual_(std::move(visual)), strategy_(std::move(strategy)), config_(std::move(config)) {}

  ~DCompVisual() {
    FML_LOG(ERROR) << "Drop visual";
  }

  ID3D12Resource* Acquire() {
    return std::visit([=](auto& strategy) { return strategy.Acquire(config_); }, strategy_);
  }

  void Present() {
    std::visit([=](auto& strategy) { strategy.Present(config_); }, strategy_);
  }

  void Resize(unsigned int width, unsigned int height) {
    auto visual = dcomp_visual_.get();
    auto& config = config_;
    config.width = width;
    config.height = height;
    std::visit([=](auto& strategy) { strategy.Resize(visual, config); }, strategy_);
  }
};

WindowSurfaceD3D12::WindowSurfaceD3D12() {}

WindowSurfaceD3D12::~WindowSurfaceD3D12() {}

static gr_cp<IDXGIAdapter1> FindIntegratedAdapter() {
  gr_cp<IDXGIAdapter1> adapter;

  gr_cp<IDXGIFactory4> factory;
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
    return nullptr;
  }

  gr_cp<IDXGIFactory6> factory6;
  if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
    if (SUCCEEDED(factory6->EnumAdapterByGpuPreference(
            0, DXGI_GPU_PREFERENCE_MINIMUM_POWER, IID_PPV_ARGS(&adapter)))) {
      return adapter;
    }
  }

  // Otherwise, take the first adapter.
  if (factory->EnumAdapters1(0, &adapter) == DXGI_ERROR_NOT_FOUND) {
    return nullptr;
  }

  return adapter;
}

std::unique_ptr<WindowSurfaceD3D12> WindowSurfaceD3D12::Create() {
  // Enable the D3D12 debug layer.
  {
    gr_cp<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
      debugController->EnableDebugLayer();
    }
  }

  gr_cp<IDXGIAdapter1> adapter = FindIntegratedAdapter();
  if (!adapter)
    return nullptr;

  gr_cp<ID3D12Device> device;
  if (FAILED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0,
                               IID_PPV_ARGS(&device)))) {
    return nullptr;
  }

  gr_cp<ID3D11Device> device11;
  if (FAILED(D3D11CreateDevice(adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                               D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
                               D3D11_SDK_VERSION, &device11, nullptr,
                               nullptr))) {
    FML_LOG(ERROR) << "Could not create D3D11Device";
  }

  gr_cp<IDXGIDevice> dxgi_device;
  if (FAILED(device11->QueryInterface(IID_PPV_ARGS(&dxgi_device)))) {
    FML_LOG(ERROR) << "Could not query DXGIDevice";
  }

  gr_cp<IDCompositionDevice> dcomp_device;
  if (FAILED(DCompositionCreateDevice(dxgi_device.get(),
                                      IID_PPV_ARGS(&dcomp_device)))) {
    FML_LOG(ERROR) << "Could not query DirectComposition device";
  }

  gr_cp<ID3D12CommandQueue> queue;
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)))) {
    FML_LOG(ERROR) << "Could not create D3D12CommandQueue";
    return nullptr;
  }

  auto surface = std::make_unique<WindowSurfaceD3D12>();
  surface->dxgi_adapter_ = adapter;
  surface->device_ = device;
  surface->queue_ = queue;
  surface->dcomp_device_ = dcomp_device;

  GR_D3D_CALL_ERRCHECK(device11->QueryInterface(IID_PPV_ARGS(&surface->device11_)));

  if (FAILED(device->CreateFence(surface->fence_value_, D3D12_FENCE_FLAG_SHARED,
                                 IID_PPV_ARGS(&surface->fence_)))) {
    FML_LOG(ERROR) << "Could not create D3D12Fence";
    return nullptr;
  }

  HANDLE handle = 0;
  GR_D3D_CALL_ERRCHECK(surface->device_->CreateSharedHandle(surface->fence_.get(), nullptr, GENERIC_ALL, nullptr, &handle));

  GR_D3D_CALL_ERRCHECK(surface->device11_->OpenSharedFence(handle, IID_PPV_ARGS(&surface->fence11_)));

  surface->fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);

  return surface;
}

std::unique_ptr<DCompVisual> WindowSurfaceD3D12::CreateVisual(
    unsigned int width,
    unsigned int height) {
  VisualConfig config(width, height);

  gr_cp<IDCompositionVisual> dcomp_visual;
  // visual->device_ = device_.get();

  GR_D3D_CALL_ERRCHECK(dcomp_device_->CreateVisual(&dcomp_visual));

  constexpr bool useSurfaceStrategy = true;

  PresentStrategy present_strategy;

  if (SurfaceStrategy::SupportsFormat(config.format)) {
    SurfaceStrategy strategy(device_.get(), device11_.get(), dcomp_device_.get());
    strategy.Resize(dcomp_visual.get(), config);

    present_strategy = PresentStrategy(std::move(strategy));
  } else {
    // TODO: Move to SwapChainStrategy::Resize
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.BufferCount = 2;
    desc.Width = config.width;
    desc.Height = config.height;
    desc.Format = config.format;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    desc.SampleDesc.Count = 1;

    gr_cp<IDXGISwapChain1> swap_chain;

    GR_D3D_CALL_ERRCHECK(dxgi_factory_->CreateSwapChainForComposition(
        queue_.get(), &desc, nullptr, &swap_chain));

    GR_D3D_CALL_ERRCHECK(dcomp_visual->SetContent(swap_chain.get()));

    SwapChainStrategy strategy;

    GR_D3D_CALL_ERRCHECK(
        swap_chain->QueryInterface(IID_PPV_ARGS(&strategy.swap_chain_)));

    present_strategy = PresentStrategy(std::move(strategy));
  }

  std::unique_ptr<DCompVisual> visual = std::make_unique<DCompVisual>(
    std::move(dcomp_visual),
    std::move(present_strategy),
    std::move(config)
  );

  return visual;
}

void WindowSurfaceD3D12::Init(PlatformWindow window,
                              unsigned int width,
                              unsigned int height) {
  if (FAILED(CreateDXGIFactory2(0,
                                IID_PPV_ARGS(&dxgi_factory_)))) {
    FML_LOG(ERROR) << "Failed to create DXGIFactory4";
    return;
  }

  gr_cp<IDXGIInfoQueue> dxgiInfoQueue;
  if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue)))) {
    dxgiInfoQueue->SetBreakOnSeverity(
        DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
    dxgiInfoQueue->SetBreakOnSeverity(
        DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
  }

  // We don't support fullscreen transitions.
  GR_D3D_CALL_ERRCHECK(
      dxgi_factory_->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));

  GR_D3D_CALL_ERRCHECK(
      dcomp_device_->CreateTargetForHwnd(window, true, &dcomp_target_));

  root_visual_ = CreateVisual(width, height);

  GR_D3D_CALL_ERRCHECK(
      dcomp_target_->SetRoot(root_visual_->dcomp_visual_.get()));

  window_ = window;

  // BOOL cloak = TRUE;
  //DwmSetWindowAttribute(static_cast<HWND>(TestingPlatformView), DWMWA_CLOAK, &cloak, sizeof(cloak));

  // FML_LOG(ERROR) << TestingPlatformView;

  // GR_D3D_CALL_ERRCHECK(dcomp_device_->CreateVisual(&platform_view_visual_));
  // GR_D3D_CALL_ERRCHECK(
  //     dcomp_device_->CreateSurfaceFromHwnd(static_cast<HWND>(TestingPlatformView), &surface_));
  // GR_D3D_CALL_ERRCHECK(platform_view_visual_->SetContent(surface_.get()));

  // GR_D3D_CALL_ERRCHECK(
  //     dcomp_target_->SetRoot(platform_view_visual_.get()));
  // GR_D3D_CALL_ERRCHECK(
  //     dcomp_device_->Commit());
}

void WindowSurfaceD3D12::Destroy() {
  window_ = nullptr;
}

void WindowSurfaceD3D12::GetSurfaceDimensions(unsigned int& width,
                                              unsigned int& height) {
  width = root_visual_->config_.width;
  height = root_visual_->config_.height;
}

static WindowSurfaceD3D12* GetWindowSurface(void* user_data) {
  auto host = static_cast<FlutterWindowsEngine*>(user_data);
  return static_cast<WindowSurfaceD3D12*>(host->surface());
}

// Creates and returns a FlutterRendererConfig that renders to the view (if any)
// of a FlutterWindowsEngine, using OpenGL (via ANGLE).
// The user_data received by the render callbacks refers to the
// FlutterWindowsEngine.
void WindowSurfaceD3D12::InitRendererConfig(FlutterRendererConfig& config) {
  config.type = kD3D12;
  config.d3d12.struct_size = sizeof(FlutterD3D12RendererConfig);
  config.d3d12.dxgi_adapter =
      static_cast<FlutterD3D12DXGIAdapter1>(dxgi_adapter_.get());
  config.d3d12.device = static_cast<FlutterD3D12DXGIAdapter1>(device_.get());
  config.d3d12.command_queue =
      static_cast<FlutterD3D12DXGIAdapter1>(queue_.get());
  config.d3d12.acquire_back_buffer_callback =
      [](void* user_data,
         const FlutterFrameInfo* info) -> FlutterD3D12Resource {
    auto surface = GetWindowSurface(user_data);

    auto hook = surface->GetWindowSurfaceHook();
    if (hook)
      hook->OnAcquireFrame(info->size.width, info->size.height);

    if (surface->fence_value_ > 0 &&
        (surface->fence_value_ - surface->fence_->GetCompletedValue()) > 1) {
      FML_LOG(ERROR) << "Stall";
      // Wait for the frame before the last frame to complete.
      GR_D3D_CALL_ERRCHECK(
          surface->fence_->SetEventOnCompletion(surface->fence_value_ - 1, surface->fence_event_));
      WaitForSingleObjectEx(surface->fence_event_, INFINITE, FALSE);
    }

    surface->fence_value_++;
    return static_cast<FlutterD3D12Resource>(surface->root_visual_->Acquire());
  };
  config.d3d12.present_callback = [](void* user_data) {
    auto surface = GetWindowSurface(user_data);
    auto hook = surface->GetWindowSurfaceHook();
    bool canceled = hook && !hook->OnPresentFramePre();

    if (!canceled) {
      surface->root_visual_->Present();
      GR_D3D_CALL_ERRCHECK(surface->dcomp_device_->Commit());
    }

    GR_D3D_CALL_ERRCHECK(
        surface->queue_->Signal(surface->fence_.get(), surface->fence_value_));

    // Intentionally done outside the scope of the lock above, as this will synchonize with all of the message queues of
    // the platform view windows.
    // Otherwise, may deadlock if a separate message queue is in the process of registering a platform view.
    if (surface->deferred_window_positions_) {
      EndDeferWindowPos(surface->deferred_window_positions_);
      surface->deferred_window_positions_ = 0;
    }

    if (canceled)
      return false;

    if (hook)
      hook->OnPresentFramePost();
    return true;
  };
}

void WindowSurfaceD3D12::CleanUpRendererConfig(FlutterRendererConfig& config) {}

void WindowSurfaceD3D12::OnResize(unsigned int width, unsigned int height) {
  // If we haven't finished rendering the frame, synchronize with the GPU fence.
  if (fence_->GetCompletedValue() < fence_value_) {
    GR_D3D_CALL_ERRCHECK(
        fence_->SetEventOnCompletion(fence_value_, fence_event_));
    WaitForSingleObjectEx(fence_event_, INFINITE, FALSE);
  }

  root_visual_->Resize(width, height);
}

bool WindowSurfaceD3D12::InitCompositorConfig(FlutterCompositor& config) {
  config.user_data = this;
  // We maintain our own cache, because the API provided by FlutterCompositor
  // does not have a callback to acquire an image.
  // TODO(38468): The optimization to elide backing store updates between frames
  // has not been implemented yet. When this is implemented, move to an acquire
  // callback.
  config.avoid_backing_store_cache = true;
  config.use_root_surface = true;
  config.create_backing_store_callback =
      [](const FlutterBackingStoreConfig* config, FlutterBackingStore* out,
         void* user_data) -> bool {
    out->struct_size = sizeof(FlutterBackingStore);
    WindowSurfaceD3D12* surface = static_cast<WindowSurfaceD3D12*>(user_data);
    std::unique_ptr<DCompVisual> visual;

    // Check for an existing matching visual in the cache.
    // We manage a singly linked list of cached visuals that are no longer in use.
    // The order of this cache matches the destruction order, so in the general case the
    // layers are reused as long as the order is mostly similar.
    // There are some limited cases where ordering changes or layer size changes may prevent
    // layer reuse.
    DCompVisual* prev_visual = nullptr;
    DCompVisual* next_visual = surface->visuals_cache_.get();
    for (int i = 0; i < 8; i++) {
      if (!next_visual)
        break;
      if (next_visual->config_.width == config->size.width &&
          next_visual->config_.height == config->size.height) {
        // Use it, take it out of the cache:
        visual = prev_visual ? std::move(prev_visual->next_)
                             : std::move(surface->visuals_cache_);
        if (prev_visual) {
          prev_visual->next_ = std::move(visual->next_);
        } else {
          surface->visuals_cache_ = std::move(visual->next_);
        }
        break;
      }
      prev_visual = next_visual;
      next_visual = next_visual->next_.get();
    }

    if (!visual) {
      visual = surface->CreateVisual(config->size.width, config->size.height);
    }

    out->type = kFlutterBackingStoreTypeD3D12;
    out->d3d12.struct_size = sizeof(FlutterD3D12BackingStore);
    out->d3d12.acquire_callback = [](void* ud, const FlutterFrameInfo* info) -> FlutterD3D12Resource {
      // TODO: We don't need info, why do we provide it?
      DCompVisual* visual = static_cast<DCompVisual*>(ud);
      visual->acquired_image_.retain(visual->Acquire());
      // FML_LOG(ERROR) << "Acquire " << visual->acquired_image_.get();
      return static_cast<FlutterD3D12Resource>(visual->acquired_image_.get());
    };
    out->d3d12.release_callback = [](void* ud) {
      DCompVisual* visual = static_cast<DCompVisual*>(ud);
      auto image = visual->acquired_image_.release();
      auto cnt = image->Release();
      // FML_LOG(ERROR) << "Destroy " << image << " " << cnt;
    };
    out->d3d12.user_data = static_cast<void*>(visual.get());
    out->user_data = static_cast<void*>(visual.release());
    return true;
  };
  config.collect_backing_store_callback = [](const FlutterBackingStore* store,
                                             void* user_data) -> bool {
    WindowSurfaceD3D12* surface = static_cast<WindowSurfaceD3D12*>(user_data);

    std::unique_ptr<DCompVisual> visual(
        static_cast<DCompVisual*>(store->user_data));

    if (surface->visuals_cache_tail_) {
      auto new_tail = visual.get();
      surface->visuals_cache_tail_->next_ = std::move(visual);
      surface->visuals_cache_tail_ = new_tail;
    } else {
      visual->next_ = std::move(surface->visuals_cache_);
      surface->visuals_cache_ = std::move(visual);
      surface->visuals_cache_tail_ = surface->visuals_cache_.get();
    }

    return true;
  };
  config.present_layers_callback = [](const FlutterLayer** layers,
                                      size_t layers_count,
                                      void* user_data) -> bool {
    WindowSurfaceD3D12* surface = static_cast<WindowSurfaceD3D12*>(user_data);

    // Synchronize between D3D11 and D3D12:
    GR_D3D_CALL_ERRCHECK(
        surface->queue_->Signal(surface->fence_.get(), surface->fence_value_));
    gr_cp<ID3D11DeviceContext> context;
    surface->device11_->GetImmediateContext(&context);
    ID3D11DeviceContext4* context4;
    GR_D3D_CALL_ERRCHECK(context->QueryInterface(IID_PPV_ARGS(&context4)));
    GR_D3D_CALL_ERRCHECK(context4->Wait(surface->fence11_.get(), surface->fence_value_));

    surface->fence_value_++;

    auto root_visual = surface->root_visual_->dcomp_visual_;
    auto& shown_visuals = surface->shown_visuals_;

    HDWP hdwp = 0;
    HWND previous_window = HWND_BOTTOM;

    {
      auto lock = surface->platform_views_->AcquireLock();

      auto prev_last_frame_visual = shown_visuals.before_begin();
      auto last_frame_visual = shown_visuals.begin();

      auto discard_visuals = [&]() {
        // Rebuild the rest of the visual list
        std::for_each(last_frame_visual, shown_visuals.end(), [&](IDCompositionVisual* visual) {
          GR_D3D_CALL_ERRCHECK(root_visual->RemoveVisual(visual));
        });
        last_frame_visual = shown_visuals.end();
        shown_visuals.erase_after(prev_last_frame_visual, last_frame_visual);
      };

      // Add visuals associated with layers.
      for (size_t i = 0; i < layers_count; i++) {
        auto layer = layers[i];
        IDCompositionVisual* raw_visual = nullptr;

        if (layer->type == kFlutterLayerContentTypeBackingStore) {
          DCompVisual* visual =
              static_cast<DCompVisual*>(layer->backing_store->user_data);
          visual->dcomp_visual_->SetOffsetX(layer->offset.x);
          visual->dcomp_visual_->SetOffsetY(layer->offset.y);
          visual->Present();
          raw_visual = visual->dcomp_visual_.get();
        } else if (layer->type == kFlutterLayerContentTypePlatformView) {
          auto view = layer->platform_view;
          auto found_view = surface->platform_views_->FindViewById(lock, view->identifier);

          if (!found_view) {
            FML_LOG(ERROR) << "View not found " << view->identifier;

            continue;
          }

          D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F::Identity();

          // TODO: How are we supposed to detect the root surface transform??
          for (size_t i=0; i<view->mutations_count; i++) {
            auto mut = view->mutations[i];
            if (mut->type == kFlutterPlatformViewMutationTypeTransformation) {
              // FML_LOG(ERROR) << "Transform: " << mut->transformation.transX << ", " << mut->transformation.transY;
              // TODO: We are ignoring perspective factor on purpose >:()
              D2D1::Matrix3x2F in_matrix;
              in_matrix.m11 = mut->transformation.scaleX;
              in_matrix.m21 = mut->transformation.skewX;
              in_matrix.dx = mut->transformation.transX;
              in_matrix.m12 = mut->transformation.skewY;
              in_matrix.m22 = mut->transformation.scaleY;
              in_matrix.dy = mut->transformation.transY;

              matrix = in_matrix * matrix;
            }
          }

          if (view->device_pixel_ratio != 1.0) {
            D2D1::Matrix3x2F dpr_matrix = D2D1::Matrix3x2F::Identity();
            dpr_matrix.m11 = dpr_matrix.m22 = 1.0 / view->device_pixel_ratio;

            matrix = dpr_matrix * matrix;
          }

          // TODO: Set clip rect (which is pre-transform)
          // D2D_MATRIX_3X2_F matrixi = D2D1::Matrix3x2F(1.0/3.0, 0, 0, 1.0/3.0, 0, 0);
          // gr_cp<IDCompositionMatrixTransform> transform;
          // surface->dcomp_device_->CreateMatrixTransform(&transform);
          // transform->SetMatrix(matrixi);
          // found_view->visual()->SetTransform(transform.get());
          found_view->visual()->SetOffsetX(matrix.dx);
          found_view->visual()->SetOffsetY(matrix.dy);
          // found_view->visual()->SetClip(D2D1::RectF(0, 0, 400 * 8, 256));
          raw_visual = found_view->visual();

          FlutterDesktopPlatformViewUpdate update;
          update.transform.scaleX = matrix.m11;
          update.transform.skewX = matrix.m21;
          update.transform.transX = matrix.dx;

          update.transform.skewY = matrix.m12;
          update.transform.scaleY = matrix.m22;
          update.transform.transY = matrix.dy;

          update.transform.pers0 = 0.0;
          update.transform.pers1 = 0.0;
          update.transform.pers2 = 1.0;
          // TODO: Epsilon in comparison here
          update.is_simple_translate = update.transform.scaleX == 1.0 && update.transform.skewX == 0.0 && update.transform.scaleY == 1.0 && update.transform.skewY == 0.0;
          update.size.width = layer->size.width;
          update.size.height = layer->size.height;

          found_view->UpdateVisible(hdwp, previous_window, update);
        }

        if (raw_visual) {
          if (last_frame_visual != shown_visuals.end() && *last_frame_visual != raw_visual) {
            discard_visuals();
          }

          if (last_frame_visual == shown_visuals.end()) {
            auto prev = prev_last_frame_visual == shown_visuals.before_begin() ?
                nullptr :
                *prev_last_frame_visual;
            GR_D3D_CALL_ERRCHECK(root_visual->AddVisual(raw_visual, prev != nullptr, prev));
            last_frame_visual = shown_visuals.insert_after(prev_last_frame_visual, raw_visual);
          }

          prev_last_frame_visual++;
          last_frame_visual++;
        }
      }

      discard_visuals();

      // Cull visuals unused in this frame.
      surface->visuals_cache_tail_ = nullptr;
      if (surface->visuals_cache_) {
        FML_LOG(ERROR) << "Disposing visuals";
      }
      surface->visuals_cache_ = nullptr;
    }

    surface->deferred_window_positions_ = hdwp;

    // TODO: Manage an invisible input glass pane above platform views. Use SetWindowRgn to control where mouse input goes.
    // Use WS_EX_NOREDIRECTIONBITMAP + DWM Cloaking to prevent graphics resources from being used.

    return true;
  };
  return true;
}

void* WindowSurfaceD3D12::GetCompositionDevice() {
  return static_cast<void*>(dcomp_device_.get());
}

}  // namespace flutter
