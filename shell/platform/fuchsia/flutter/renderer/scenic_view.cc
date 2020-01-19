// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_view.h"

#include <fuchsia/images/cpp/fidl.h>
#include <lib/images/cpp/images.h>
#include <lib/ui/scenic/cpp/commands.h>

#include <algorithm>
#include <limits>

#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace flutter_runner {

ScenicView::ScenicView(const std::string& debug_label,
                       ScenicSession& session,
                       fuchsia::ui::views::ViewToken view_token,
                       scenic::ViewRefPair view_ref_pair)
    : session_(session),
      root_view_(session_.get(),
                 std::move(view_token),
                 std::move(view_ref_pair.control_ref),
                 std::move(view_ref_pair.view_ref),
                 debug_label),
      root_node_(session_.get()),
      scale_node_(session_.get()) {
  root_node_.SetEventMask(fuchsia::ui::gfx::kMetricsEventMask);
  root_node_.AddChild(scale_node_);
  root_view_.AddChild(root_node_);
  session_.QueuePresent();
}

bool ScenicView::CreateBackingStore(
    const FlutterBackingStoreConfig* layer_config,
    FlutterBackingStore* backing_store_out) {
  FX_LOG(ERROR) << "CREATE BACKING STORE " << layer_config->size.width << " "
                << layer_config->size.height;

  uint32_t store_width = static_cast<uint32_t>(layer_config->size.width);
  uint32_t store_height = static_cast<uint32_t>(layer_config->size.height);
  auto cached_store = std::find_if(
      backing_stores_.begin(), backing_stores_.end(),
      [store_width, store_height](auto& backing_store) -> bool {
        return backing_store && backing_store->width == store_width &&
               backing_store->height == store_height && backing_store->age == 0;
      });

  if (cached_store != backing_stores_.end()) {
    FX_DCHECK(*cached_store);

    size_t backing_store_index = cached_store - backing_stores_.begin();
    *backing_store_out = FlutterBackingStore{
        .struct_size = sizeof(FlutterBackingStore),
        .user_data = reinterpret_cast<void*>(backing_store_index),
        .type = kFlutterBackingStoreTypeSoftware,
        .did_update = true,
        .software =
            FlutterSoftwareBackingStore{
                .allocation = (*cached_store)->base_address,
                .row_bytes = (*cached_store)->stride,
                .height = (*cached_store)->height,
                .user_data = reinterpret_cast<void*>(backing_store_index),
                .destruction_callback =
                    [](void* user_data) {
                      FX_LOG(ERROR)
                          << "BACKING STORE SKIA DESTRUCTION CALLBACK";
                    },
            },
    };
  } else {
    constexpr fuchsia::images::PixelFormat kFormat =
        fuchsia::images::PixelFormat::BGRA_8;
    uint32_t store_stride =
        store_width * images::StrideBytesPerWidthPixel(kFormat);
    fuchsia::images::ImageInfo image_info{
        .transform = fuchsia::images::Transform::NORMAL,
        .width = store_width,
        .height = store_height,
        .stride = store_stride,
        .pixel_format = kFormat,
        .color_space = fuchsia::images::ColorSpace::SRGB,
        .tiling = fuchsia::images::Tiling::LINEAR,
        .alpha_format = fuchsia::images::AlphaFormat::PREMULTIPLIED,
    };
    uint64_t image_vmo_bytes = images::ImageSize(image_info);

    zx::vmo image_vmo;
    zx_status_t create_status = zx::vmo::create(image_vmo_bytes, 0, &image_vmo);
    if (create_status != ZX_OK) {
      FX_LOG(FATAL) << "zx::vmo::create() failed";
    }

    const void* vmo_base;
    zx_status_t map_status = zx::vmar::root_self()->map(
        0, image_vmo, 0, image_vmo_bytes, ZX_VM_PERM_WRITE | ZX_VM_PERM_READ,
        reinterpret_cast<uintptr_t*>(&vmo_base));
    if (map_status != ZX_OK) {
      FX_LOG(FATAL) << "zx::vmar::map() failed";
    }

    auto memory = std::make_unique<scenic::Memory>(
        session_.get(), std::move(image_vmo), image_vmo_bytes,
        fuchsia::images::MemoryType::HOST_MEMORY);
    auto image = std::make_unique<scenic::Image>(session_.get(), memory->id(),
                                                 0, image_info);

    size_t backing_store_index;
    auto empty_store = std::find_if(
        backing_stores_.begin(), backing_stores_.end(),
        [](auto& backing_store) -> bool { return !backing_store; });
    if (empty_store != backing_stores_.end()) {
      backing_store_index = empty_store - backing_stores_.begin();
      FX_DCHECK(backing_store_index < backing_stores_.size());

      backing_stores_.emplace(empty_store, BackingStore{
                                               .memory = std::move(memory),
                                               .image = std::move(image),
                                               .base_address = vmo_base,
                                               .width = store_width,
                                               .stride = store_stride,
                                               .height = store_height,
                                               .age = 0,
                                           });
    } else {
      backing_store_index = backing_stores_.size();
      backing_stores_.emplace_back(BackingStore{
          .memory = std::move(memory),
          .image = std::move(image),
          .base_address = vmo_base,
          .width = store_width,
          .stride = store_stride,
          .height = store_height,
          .age = 0,
      });
    }

    *backing_store_out = FlutterBackingStore{
        .struct_size = sizeof(FlutterBackingStore),
        .user_data = reinterpret_cast<void*>(backing_store_index),
        .type = kFlutterBackingStoreTypeSoftware,
        .did_update = true,
        .software =
            FlutterSoftwareBackingStore{
                .allocation = vmo_base,
                .row_bytes = image_info.stride,
                .height = image_info.height,
                .user_data = reinterpret_cast<void*>(backing_store_index),
                .destruction_callback =
                    [](void* user_data) {
                      FX_LOG(ERROR)
                          << "BACKING STORE SKIA DESTRUCTION CALLBACK";
                    },
            },
    };
  }

  return true;
}

bool ScenicView::CollectBackingStore(const FlutterBackingStore* backing_store) {
  FX_DCHECK(backing_store->type == kFlutterBackingStoreTypeSoftware);
  size_t backing_store_index =
      reinterpret_cast<size_t>(backing_store->software.user_data);
  FX_DCHECK(backing_store_index < backing_stores_.size());
  FX_LOG(ERROR) << "COLLECT BACKING STORE FOR " << backing_store_index;

  // TODO(dworsham): Mark unused so we can reclaim

  return false;
}

bool ScenicView::PresentLayers(const FlutterLayer** layers,
                               size_t layer_count) {
  FX_DCHECK(layer_count == 1);
  FX_LOG(ERROR) << "PRESENT LAYERS";
  FX_LOG(ERROR) << "LAYER 0 OFFSET=" << layers[0]->offset.x << ","
                << layers[0]->offset.y << " SIZE=" << layers[0]->size.width
                << "," << layers[0]->size.height;

  // Age out any backing stores that are currently in use by Scenic.  Backing
  // stores get 3 frames of usage by Scenic before Flutter may re-use them.
  for (auto& backing_store : backing_stores_) {
    if (backing_store && backing_store->age != 0) {
      backing_store->age = (backing_store->age + 1) % 8;
    }
  }

  if (layers_.empty()) {
    layers_.emplace_back(Layer{
        .material = scenic::Material(session_.get()),
        .shape = scenic::ShapeNode(session_.get()),
    });

    layers_[0].shape.SetMaterial(layers_[0].material);
    layers_[0].shape.SetShape(scenic::Rectangle(
        session_.get(), layers[0]->size.width, layers[0]->size.height));
    layers_[0].shape.SetTranslation(
        0.5f * layers[0]->size.width + layers[0]->offset.x,
        0.5f * layers[0]->size.height + layers[0]->offset.y,
        std::numeric_limits<float>::epsilon());
    scale_node_.AddChild(layers_[0].shape);
  }

  FX_DCHECK(layers[0]->type == kFlutterLayerContentTypeBackingStore);
  FX_DCHECK(layers[0]->backing_store->type == kFlutterBackingStoreTypeSoftware);
  size_t backing_store_index =
      reinterpret_cast<size_t>(layers[0]->backing_store->software.user_data);
  FX_DCHECK(backing_store_index < backing_stores_.size());
  auto& internal_backing_store = backing_stores_[backing_store_index];
  FX_DCHECK(internal_backing_store);
  FX_DCHECK(internal_backing_store->age == 0);
  internal_backing_store->age = 1;  // Mark as in-use by Scenic.

  auto* image = internal_backing_store->image.get();
  FX_DCHECK(image);
  layers_[0].material.SetTexture(*image);

  session_.QueuePresent();

  return true;
}

bool ScenicView::IsBackingStoreAvailable(
    const FlutterBackingStore* backing_store) {
  FX_DCHECK(backing_store->type == kFlutterBackingStoreTypeSoftware);
  size_t backing_store_index =
      reinterpret_cast<size_t>(backing_store->software.user_data);
  FX_DCHECK(backing_store_index < backing_stores_.size());
  FX_LOG(ERROR) << "IS BACKING STORE AVAILABLE FOR " << backing_store_index;
  auto& internal_backing_store = backing_stores_[backing_store_index];
  FX_DCHECK(internal_backing_store);

  FX_LOG(ERROR) << "  AGE " << internal_backing_store->age << " -- "
                << (internal_backing_store->age == 0 ? "AVAILABLE"
                                                     : "UNAVAILABLE");

  return (internal_backing_store->age == 0);
}

void ScenicView::OnChildViewConnected(scenic::ResourceId view_id) {
  FX_DCHECK(false);
}

void ScenicView::OnChildViewDisconnected(scenic::ResourceId view_id) {
  FX_DCHECK(false);
}

void ScenicView::OnChildViewStateChanged(scenic::ResourceId view_id,
                                         bool is_rendering) {
  FX_DCHECK(false);
}

void ScenicView::OnPixelRatioChanged(float pixel_ratio) {
  FX_LOG(ERROR) << "PIXEL RATIO CHANGED " << pixel_ratio;
  scale_node_.SetScale(1.0f / pixel_ratio, 1.0f / pixel_ratio, 1.0f);
}

void ScenicView::OnEnableWireframe(bool enable) {
  session_.get()->Enqueue(
      scenic::NewSetEnableDebugViewBoundsCmd(root_view_.id(), enable));
}

}  // namespace flutter_runner
