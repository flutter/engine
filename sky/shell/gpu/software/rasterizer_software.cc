// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/gpu/software/rasterizer_software.h"

#include "base/trace_event/trace_event.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefPtr.h"
#include "sky/shell/platform_view.h"
#include "sky/shell/shell.h"
#include "third_party/skia/include/core/SkBitmapDevice.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace sky {
namespace shell {

RasterizerSoftware::RasterizerSoftware()
    : binding_(this), weak_factory_(this) {}

RasterizerSoftware::~RasterizerSoftware() {
  weak_factory_.InvalidateWeakPtrs();
  Shell::Shared().PurgeRasterizers();
}

// sky::shell::Rasterizer override.
base::WeakPtr<Rasterizer> RasterizerSoftware::GetWeakRasterizerPtr() {
  return weak_factory_.GetWeakPtr();
}

// sky::shell::Rasterizer override.
void RasterizerSoftware::ConnectToRasterizer(
    mojo::InterfaceRequest<rasterizer::Rasterizer> request) {
  binding_.Bind(request.Pass());

  Shell::Shared().AddRasterizer(GetWeakRasterizerPtr());
}

// sky::shell::Rasterizer override.
void RasterizerSoftware::Setup(PlatformView* platform_view,
                               base::Closure continuation,
                               base::WaitableEvent* setup_completion_event) {
  CHECK(platform_view) << "Must be able to acquire the view.";
  platform_view_ = platform_view;
  continuation.Run();
  setup_completion_event->Signal();
}

// sky::shell::Rasterizer override.
void RasterizerSoftware::Teardown(
    base::WaitableEvent* teardown_completion_event) {
  platform_view_ = nullptr;
  teardown_completion_event->Signal();
}

// sky::shell::Rasterizer override.
flow::LayerTree* RasterizerSoftware::GetLastLayerTree() {
  return last_layer_tree_.get();
}

void RasterizerSoftware::Draw(uint64_t layer_tree_ptr,
                              const DrawCallback& callback) {
  TRACE_EVENT0("flutter", "RasterizerSoftware::Draw");

  std::unique_ptr<flow::LayerTree> layer_tree(
      reinterpret_cast<flow::LayerTree*>(layer_tree_ptr));

  if (platform_view_ == nullptr || !layer_tree->root_layer()) {
    callback.Run();
    return;
  }

  SkISize size = layer_tree->frame_size();

  // There is no way for the compositor to know how long the layer tree
  // construction took. Fortunately, the layer tree does. Grab that time
  // for instrumentation.
  compositor_context_.engine_time().SetLapTime(layer_tree->construction_time());

  {
    SkImageInfo info = SkImageInfo::MakeN32Premul(size.width(), size.height());
    sk_sp<SkBitmapDevice> device(SkBitmapDevice::Create(info));
    sk_sp<SkCanvas> canvas = sk_make_sp<SkCanvas>(device.get());

    flow::CompositorContext::ScopedFrame frame =
        compositor_context_.AcquireFrame(nullptr, *canvas);
    canvas->clear(SK_ColorBLACK);
    layer_tree->Raster(frame);
    canvas->flush();

    // TODO(abarth): Swap buffers.
  }

  callback.Run();
  last_layer_tree_ = std::move(layer_tree);
}

}  // namespace shell
}  // namespace sky
