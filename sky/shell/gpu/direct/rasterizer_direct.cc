// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/gpu/direct/rasterizer_direct.h"

#include "base/trace_event/trace_event.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefPtr.h"
#include "sky/shell/gpu/picture_serializer.h"
#include "sky/shell/shell.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace sky {
namespace shell {

RasterizerDirect::RasterizerDirect() : binding_(this), weak_factory_(this) {}

RasterizerDirect::~RasterizerDirect() {
  weak_factory_.InvalidateWeakPtrs();
  Shell::Shared().PurgeRasterizers();
}

// Implementation of declaration in sky/shell/rasterizer.h.
std::unique_ptr<Rasterizer> Rasterizer::Create() {
  return std::unique_ptr<Rasterizer>(new RasterizerDirect());
}

// sky::shell::Rasterizer override.
base::WeakPtr<Rasterizer> RasterizerDirect::GetWeakRasterizerPtr() {
  return weak_factory_.GetWeakPtr();
}

// sky::shell::Rasterizer override.
void RasterizerDirect::ConnectToRasterizer(
    mojo::InterfaceRequest<rasterizer::Rasterizer> request) {
  binding_.Bind(request.Pass());

  Shell::Shared().AddRasterizer(GetWeakRasterizerPtr());
}

// sky::shell::Rasterizer override.
void RasterizerDirect::Setup(base::WeakPtr<PlatformView> delegate) {
  ganesh_canvas_.SetupGrGLInterface();
  view_delegate_ = delegate;
}

// sky::shell::Rasterizer override.
void RasterizerDirect::Teardown() {
  view_delegate_.reset();
}

// sky::shell::Rasterizer override.
flow::LayerTree* RasterizerDirect::GetLastLayerTree() {
  return last_layer_tree_.get();
}

void RasterizerDirect::Draw(uint64_t layer_tree_ptr,
                            const DrawCallback& callback) {
  TRACE_EVENT0("flutter", "RasterizerDirect::Draw");

  std::unique_ptr<flow::LayerTree> layer_tree(
      reinterpret_cast<flow::LayerTree*>(layer_tree_ptr));

  auto view = view_delegate_.get();

  if (view == nullptr || !layer_tree->root_layer()) {
    callback.Run();
    return;
  }

  SkISize size = layer_tree->frame_size();

  // There is no way for the compositor to know how long the layer tree
  // construction took. Fortunately, the layer tree does. Grab that time
  // for instrumentation.
  compositor_context_.engine_time().SetLapTime(layer_tree->construction_time());

  {
    SkCanvas* canvas = ganesh_canvas_.GetCanvas(
        0 /* Default window bounds framebuffer */, layer_tree->frame_size());
    flow::CompositorContext::ScopedFrame frame =
        compositor_context_.AcquireFrame(ganesh_canvas_.gr_context(), *canvas);
    canvas->clear(SK_ColorBLACK);
    layer_tree->Raster(frame);
    canvas->flush();

    view->SwapBuffers();
  }

  // Trace to a file if necessary
  static const double kOneFrameDuration = 1e3 / 60.0;
  bool frameExceededThreshold = false;
  uint32_t thresholdInterval = layer_tree->rasterizer_tracing_threshold();
  if (thresholdInterval != 0 &&
      compositor_context_.frame_time().LastLap().InMillisecondsF() >
          thresholdInterval * kOneFrameDuration) {
    // While rendering the last frame, if we exceeded the tracing threshold
    // specified in the layer tree, we force a trace to disk.
    frameExceededThreshold = true;
  }

  const auto& tracingController = Shell::Shared().tracing_controller();

  if (frameExceededThreshold || tracingController.picture_tracing_enabled()) {
    base::FilePath path = tracingController.PictureTracingPathForCurrentTime();

    SkPictureRecorder recoder;
    recoder.beginRecording(SkRect::MakeWH(size.width(), size.height()));

    {
      auto frame = compositor_context_.AcquireFrame(
          nullptr, *recoder.getRecordingCanvas(), false);
      layer_tree->Raster(frame);
    }

    sk_sp<SkPicture> picture = recoder.finishRecordingAsPicture();
    SerializePicture(path, picture.get());
  }

  callback.Run();

  last_layer_tree_ = std::move(layer_tree);
}

}  // namespace shell
}  // namespace sky
