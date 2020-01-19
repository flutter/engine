// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_view_controller.h"

#include <lib/ui/scenic/cpp/commands.h>

#include "flutter/fml/logging.h"

namespace flutter_runner {
namespace {

fuchsia::ui::views::ViewRef CloneViewRef(
    const fuchsia::ui::views::ViewRef& view_ref) {
  fuchsia::ui::views::ViewRef new_view_ref;
  view_ref.Clone(&new_view_ref);

  return new_view_ref;
}

}  // namespace

ScenicViewController::ScenicViewController(
    ScenicSession& session,
    fuchsia::ui::views::ViewToken view_token)
    : ScenicViewController(session,
                           std::move(view_token),
                           scenic::ViewRefPair::New()) {}

ScenicViewController::ScenicViewController(
    ScenicSession& session,
    fuchsia::ui::views::ViewToken view_token,
    scenic::ViewRefPair view_ref_pair)
    : session_(session),
      root_view_ref_(CloneViewRef(view_ref_pair.view_ref)),
      root_view_(&session_.raw(),
                 std::move(view_token),
                 std::move(view_ref_pair.control_ref),
                 std::move(view_ref_pair.view_ref),
                 "FlutterRoot"),
      root_node_(&session_.raw()) {
  root_view_.AddChild(root_node_);
  root_node_.SetEventMask(fuchsia::ui::gfx::kMetricsEventMask);
  session_.QueuePresent();
}

ScenicViewController::~ScenicViewController() = default;

fuchsia::ui::views::ViewRef ScenicViewController::view_ref_copy() const {
  return CloneViewRef(root_view_ref_);
}

bool ScenicViewController::CreateBackingStore(
    const FlutterBackingStoreConfig* layer_config,
    FlutterBackingStore* backing_store_out) {
  FML_CHECK(false) << "CreateBackingStore";
  return false;
}

bool ScenicViewController::CollectBackingStore(
    const FlutterBackingStore* backing_store) {
  FML_CHECK(false) << "CollectBackingStore";
  return false;
}

bool ScenicViewController::PresentLayers(const FlutterLayer** layers,
                                         size_t layer_count) {
  FML_CHECK(false) << "PresentLayers";
  return false;
}

void ScenicViewController::OnMetricsChanged(
    fuchsia::ui::gfx::Metrics new_metrics) {
  FML_DCHECK(false) << "OnMetricsChanged";
}

void ScenicViewController::OnChildViewConnected(scenic::ResourceId view_id) {
  FML_DCHECK(false) << "OnChildViewConnected";
}

void ScenicViewController::OnChildViewDisconnected(scenic::ResourceId view_id) {
  FML_DCHECK(false) << "OnChildViewDisconnected";
}

void ScenicViewController::OnChildViewStateChanged(scenic::ResourceId view_id,
                                                   bool is_rendering) {
  FML_DCHECK(false) << "OnChildViewStateChanged";
}

void ScenicViewController::OnEnableWireframe(bool enable) {
  session_.raw().Enqueue(
      scenic::NewSetEnableDebugViewBoundsCmd(root_view_.id(), enable));
}

}  // namespace flutter_runner
