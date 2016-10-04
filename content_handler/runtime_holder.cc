// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/content_handler/runtime_holder.h"

#include <utility>

#include "flutter/assets/zip_asset_bundle.h"
#include "flutter/common/threads.h"
#include "flutter/content_handler/rasterizer.h"
#include "flutter/lib/ui/mojo_services.h"
#include "flutter/runtime/asset_font_selector.h"
#include "flutter/runtime/dart_controller.h"
#include "flutter/services/engine/sky_engine.mojom.h"
#include "lib/ftl/functional/make_copyable.h"
#include "lib/ftl/functional/make_runnable.h"
#include "lib/ftl/logging.h"
#include "lib/ftl/time/time_delta.h"
#include "lib/zip/create_unzipper.h"
#include "mojo/public/cpp/application/connect.h"

namespace flutter_content_handler {
namespace {

constexpr char kSnapshotKey[] = "snapshot_blob.bin";
constexpr int kPipelineDepth = 3;

}  // namespace

RuntimeHolder::RuntimeHolder()
    : viewport_metrics_(sky::ViewportMetrics::New()),
      view_listener_binding_(this),
      weak_factory_(this) {}

RuntimeHolder::~RuntimeHolder() {
  blink::Threads::Gpu()->PostTask(
      ftl::MakeCopyable([rasterizer = std::move(rasterizer_)](){
          // Deletes rasterizer.
      }));
}

void RuntimeHolder::Init(mojo::ApplicationConnectorPtr connector,
                         std::vector<char> bundle) {
  FTL_DCHECK(!rasterizer_);
  rasterizer_.reset(new Rasterizer());
  InitRootBundle(std::move(bundle));

  mojo::ConnectToService(connector.get(), "mojo:view_manager_service",
                         mojo::GetProxy(&view_manager_));
}

void RuntimeHolder::CreateView(
    const std::string& script_uri,
    mojo::InterfaceRequest<mozart::ViewOwner> view_owner_request,
    mojo::InterfaceRequest<mojo::ServiceProvider> services) {
  if (view_listener_binding_.is_bound()) {
    // TODO(jeffbrown): Support multiple view instances (or not).
    FTL_LOG(ERROR) << "The view has already been created.";
    return;
  }

  std::vector<uint8_t> snapshot;
  if (!asset_store_->GetAsBuffer(kSnapshotKey, &snapshot)) {
    FTL_LOG(ERROR) << "Unable to load snapshot from root bundle.";
    return;
  }

  mozart::ViewListenerPtr view_listener;
  view_listener_binding_.Bind(mojo::GetProxy(&view_listener));
  view_manager_->CreateView(mojo::GetProxy(&view_),
                            std::move(view_owner_request),
                            std::move(view_listener), script_uri);

  mozart::ScenePtr scene;
  view_->CreateScene(mojo::GetProxy(&scene));
  blink::Threads::Gpu()->PostTask(ftl::MakeCopyable([
    rasterizer = rasterizer_.get(), scene = std::move(scene)
  ]() mutable { rasterizer->SetScene(std::move(scene)); }));

  runtime_ = blink::RuntimeController::Create(this);
  runtime_->CreateDartController(script_uri);
  runtime_->SetViewportMetrics(viewport_metrics_);
  runtime_->dart_controller()->RunFromSnapshot(snapshot.data(),
                                               snapshot.size());
}

void RuntimeHolder::ScheduleFrame() {
  if (pending_invalidation_)
    return;
  pending_invalidation_ = true;
  view_->Invalidate();
}

void RuntimeHolder::Render(std::unique_ptr<flow::LayerTree> layer_tree) {
  if (!is_ready_to_draw_)
    return;  // Only draw once per frame.
  is_ready_to_draw_ = false;

  layer_tree->set_frame_size(SkISize::Make(viewport_metrics_->physical_width,
                                           viewport_metrics_->physical_height));
  layer_tree->set_scene_version(scene_version_);

  blink::Threads::Gpu()->PostTask(ftl::MakeCopyable([
    rasterizer = rasterizer_.get(), layer_tree = std::move(layer_tree),
    self = GetWeakPtr()
  ]() mutable {
    rasterizer->Draw(std::move(layer_tree), [self]() {
      if (self)
        self->OnFrameComplete();
    });
  }));
}

void RuntimeHolder::DidCreateMainIsolate(Dart_Isolate isolate) {
  blink::MojoServices::Create(isolate, nullptr, nullptr,
                              std::move(root_bundle_));

  if (asset_store_)
    blink::AssetFontSelector::Install(asset_store_);
}

void RuntimeHolder::InitRootBundle(std::vector<char> bundle) {
  root_bundle_data_ = std::move(bundle);
  asset_store_ = ftl::MakeRefCounted<blink::ZipAssetStore>(
      GetUnzipperProviderForRootBundle(), blink::Threads::IO());
  new blink::ZipAssetBundle(mojo::GetProxy(&root_bundle_), asset_store_);
}

blink::UnzipperProvider RuntimeHolder::GetUnzipperProviderForRootBundle() {
  return [self = GetWeakPtr()]() {
    if (!self)
      return zip::UniqueUnzipper();
    // TODO(abarth): The lifetimes aren't quite right here. The unzipper we
    // create here might be passed off to an UnzipJob that runs on a background
    // thread. The UnzipJob might outlive this object and be referencing a dead
    // root_bundle_data_.
    return zip::CreateUnzipper(&self->root_bundle_data_);
  };
}

void RuntimeHolder::OnInvalidation(mozart::ViewInvalidationPtr invalidation,
                                   const OnInvalidationCallback& callback) {
  FTL_DCHECK(invalidation);
  pending_invalidation_ = false;

  // Apply view property changes.
  if (invalidation->properties) {
    view_properties_ = std::move(invalidation->properties);
    viewport_metrics_->physical_width =
        view_properties_->view_layout->size->width;
    viewport_metrics_->physical_height =
        view_properties_->view_layout->size->height;
    viewport_metrics_->device_pixel_ratio =
        view_properties_->display_metrics->device_pixel_ratio;
    runtime_->SetViewportMetrics(viewport_metrics_);
  }

  // Remember the scene version for rendering.
  scene_version_ = invalidation->scene_version;

  // TODO(jeffbrown): Flow the frame time through the rendering pipeline.
  if (outstanding_requests_ >= kPipelineDepth) {
    did_defer_frame_request_ = true;
  } else {
    ++outstanding_requests_;
    BeginFrame();
  }

  callback.Run();
}

ftl::WeakPtr<RuntimeHolder> RuntimeHolder::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void RuntimeHolder::BeginFrame() {
  FTL_DCHECK(outstanding_requests_ > 0);
  FTL_DCHECK(outstanding_requests_ <= kPipelineDepth) << outstanding_requests_;

  FTL_DCHECK(!is_ready_to_draw_);
  is_ready_to_draw_ = true;
  runtime_->BeginFrame(ftl::TimePoint::Now());
  const bool was_ready_to_draw = is_ready_to_draw_;
  is_ready_to_draw_ = false;

  // If we were still ready to draw when done with the frame, that means we
  // didn't draw anything this frame and we should acknowledge the frame
  // ourselves instead of waiting for the rasterizer to acknowledge it.
  if (was_ready_to_draw)
    OnFrameComplete();
}

void RuntimeHolder::OnFrameComplete() {
  FTL_DCHECK(outstanding_requests_ > 0);
  --outstanding_requests_;

  if (did_defer_frame_request_) {
    did_defer_frame_request_ = false;
    view_->Invalidate();
  }
}

}  // namespace flutter_content_handler
