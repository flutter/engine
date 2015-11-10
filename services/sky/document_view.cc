// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/sky/document_view.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_util.h"
#include "base/thread_task_runner_handle.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/converters/input_events/input_events_type_converters.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "services/asset_bundle/asset_unpacker_job.h"
#include "services/sky/compositor/layer_host.h"
#include "services/sky/compositor/rasterizer_bitmap.h"
#include "services/sky/compositor/rasterizer_ganesh.h"
#include "services/sky/compositor/texture_layer.h"
#include "services/sky/converters/input_event_types.h"
#include "services/sky/dart_library_provider_impl.h"
#include "services/sky/internals.h"
#include "services/sky/runtime_flags.h"
#include "skia/ext/refptr.h"
#include "sky/compositor/paint_context.h"
#include "sky/engine/public/platform/Platform.h"
#include "sky/engine/public/platform/WebInputEvent.h"
#include "sky/engine/public/web/Sky.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkDevice.h"

using mojo::asset_bundle::AssetUnpackerJob;

namespace sky {
namespace {

const char kSnapshotKey[] = "snapshot_blob.bin";

}  // namespace

DocumentView::DocumentView(
    mojo::InterfaceRequest<mojo::ServiceProvider> exported_services,
    mojo::ServiceProviderPtr imported_services,
    mojo::URLResponsePtr response,
    mojo::Shell* shell,
    const mojo::ui::ViewProvider::CreateViewCallback& callback)
    : response_(response.Pass()),
      exported_services_(exported_services.Pass()),
      imported_services_(imported_services.Pass()),
      shell_(shell),
      bitmap_rasterizer_(nullptr),
      view_binding_(this),
      device_pixel_ratio_(0),
      weak_factory_(this) {
  mojo::ServiceProviderPtr view_manager_provider;
  shell_->ConnectToApplication("mojo:view_manager_service",
                               mojo::GetProxy(&view_manager_provider), nullptr);
  mojo::ConnectToService(view_manager_provider.get(), &view_manager_);
  view_manager_.set_connection_error_handler(
      base::Bind(&DocumentView::OnViewConnectionError, base::Unretained(this)));

  mojo::ui::ViewPtr view;
  view_binding_.Bind(mojo::GetProxy(&view));
  view_manager_->RegisterView(view.Pass(), mojo::GetProxy(&view_host_),
                              callback);

  InitServiceRegistry();
  Load(response_.Pass());
}

DocumentView::~DocumentView() {}

base::WeakPtr<DocumentView> DocumentView::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void DocumentView::OnViewConnectionError() {
  delete this;
}

void DocumentView::LoadFromSnapshotStream(
    String name,
    mojo::ScopedDataPipeConsumerHandle snapshot) {
  if (sky_view_) {
    sky_view_->RunFromSnapshot(name, snapshot.Pass());
  }
}

void DocumentView::Load(mojo::URLResponsePtr response) {
  sky_view_ = blink::SkyView::Create(this);
  layer_host_.reset(new LayerHost(this));
  root_layer_ = make_scoped_refptr(new TextureLayer(this));
  root_layer_->set_rasterizer(CreateRasterizer());
  layer_host_->SetRootLayer(root_layer_);

  String name = String::fromUTF8(response->url);
  sky_view_->CreateView(name);
  AssetUnpackerJob* unpacker =
      new AssetUnpackerJob(mojo::GetProxy(&root_bundle_),
                           base::MessageLoop::current()->task_runner());
  unpacker->Unpack(response->body.Pass());
  root_bundle_->GetAsStream(kSnapshotKey,
                            base::Bind(&DocumentView::LoadFromSnapshotStream,
                                       weak_factory_.GetWeakPtr(), name));
}

scoped_ptr<Rasterizer> DocumentView::CreateRasterizer() {
  if (!RuntimeFlags::Get().testing())
    return make_scoped_ptr(new RasterizerGanesh(layer_host_.get()));
  // TODO(abarth): If we have more than one layer, we'll need to re-think how
  // we capture pixels for testing;
  DCHECK(!bitmap_rasterizer_);
  bitmap_rasterizer_ = new RasterizerBitmap(layer_host_.get());
  return make_scoped_ptr(bitmap_rasterizer_);
}

void DocumentView::GetPixelsForTesting(std::vector<unsigned char>* pixels) {
  DCHECK(RuntimeFlags::Get().testing()) << "Requires testing runtime flag";
  DCHECK(root_layer_) << "The root layer owns the rasterizer";
  return bitmap_rasterizer_->GetPixelsForTesting(pixels);
}

mojo::ScopedMessagePipeHandle DocumentView::TakeRootBundleHandle() {
  return root_bundle_.PassInterface().PassHandle();
}

mojo::ScopedMessagePipeHandle DocumentView::TakeServicesProvidedToEmbedder() {
  // TODO(jeffbrown): Stubbed out until we migrate from native viewport
  // to a new view system that supports embedding again.
  return mojo::ScopedMessagePipeHandle();
}

mojo::ScopedMessagePipeHandle DocumentView::TakeServicesProvidedByEmbedder() {
  // TODO(jeffbrown): Stubbed out until we migrate from native viewport
  // to a new view system that supports embedding again.
  return mojo::ScopedMessagePipeHandle();
}

mojo::ScopedMessagePipeHandle DocumentView::TakeServiceRegistry() {
  return service_registry_.PassInterface().PassHandle();
}

mojo::Shell* DocumentView::GetShell() {
  return shell_;
}

void DocumentView::BeginFrame(base::TimeTicks frame_time) {
  if (sky_view_) {
    std::unique_ptr<compositor::LayerTree> layer_tree =
        sky_view_->BeginFrame(frame_time);
    if (layer_tree)
      current_layer_tree_ = std::move(layer_tree);
    root_layer_->SetSize(sky_view_->display_metrics().physical_size);
  }
}

void DocumentView::OnSurfaceIdAvailable(mojo::SurfaceIdPtr surface_id) {
  surface_id_ = surface_id.Pass();
  if (pending_layout_callback_.is_null()) {
    view_host_->RequestLayout();
  } else {
    FinishLayout();
  }
}

void DocumentView::PaintContents(SkCanvas* canvas, const gfx::Rect& clip) {
  if (current_layer_tree_) {
    compositor::PaintContext::ScopedFrame frame =
        paint_context_.AcquireFrame(*canvas);
    current_layer_tree_->root_layer()->Paint(frame);
  }
}

void DocumentView::DidCreateIsolate(Dart_Isolate isolate) {
  Internals::Create(isolate, this);
}

mojo::NavigatorHost* DocumentView::NavigatorHost() {
  return navigator_host_.get();
}

void DocumentView::OnLayout(mojo::ui::ViewLayoutParamsPtr layout_params,
                            mojo::Array<uint32_t> children_needing_layout,
                            const OnLayoutCallback& callback) {
  DCHECK(sky_view_);

  size_.width = layout_params->constraints->max_width;
  size_.height = layout_params->constraints->max_height;
  device_pixel_ratio_ = layout_params->device_pixel_ratio;

  blink::SkyDisplayMetrics metrics;
  metrics.physical_size = blink::WebSize(size_.width, size_.height);
  metrics.device_pixel_ratio = layout_params->device_pixel_ratio;
  sky_view_->SetDisplayMetrics(metrics);

  pending_layout_callback_ = callback;
  FinishLayout();
}

void DocumentView::FinishLayout() {
  if (pending_layout_callback_.is_null() || !surface_id_.get())
    return;

  mojo::ui::ViewLayoutInfoPtr info = mojo::ui::ViewLayoutInfo::New();
  info->size = size_.Clone();
  info->surface_id = surface_id_->Clone();
  pending_layout_callback_.Run(info.Pass());
  pending_layout_callback_.reset();
}

void DocumentView::OnChildUnavailable(
    uint32_t child_key,
    const OnChildUnavailableCallback& callback) {}

void DocumentView::HandleInputEvent(mojo::EventPtr event) {
  scoped_ptr<blink::WebInputEvent> web_event =
      ConvertEvent(event, device_pixel_ratio_);
  if (!web_event)
    return;

  if (sky_view_)
    sky_view_->HandleInputEvent(*web_event);
}

void DocumentView::StartDebuggerInspectorBackend() {
  // FIXME: Do we need this for dart?
}

void DocumentView::InitServiceRegistry() {
  if (imported_services_)
    mojo::ConnectToService(imported_services_.get(), &service_registry_);
}

void DocumentView::ScheduleFrame() {
  DCHECK(sky_view_);
  layer_host_->SetNeedsAnimate();
}

void DocumentView::Render(std::unique_ptr<compositor::LayerTree> layer_tree) {
}

}  // namespace sky
