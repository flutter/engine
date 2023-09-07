// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_layers.h"

#include <algorithm>

namespace flutter {

EmbedderLayers::EmbedderLayers(DlISize frame_size,
                               double device_pixel_ratio,
                               DlTransform root_surface_transformation)
    : frame_size_(frame_size),
      device_pixel_ratio_(device_pixel_ratio),
      root_surface_transformation_(root_surface_transformation) {}

EmbedderLayers::~EmbedderLayers() = default;

void EmbedderLayers::PushBackingStoreLayer(const FlutterBackingStore* store) {
  FlutterLayer layer = {};

  layer.struct_size = sizeof(FlutterLayer);
  layer.type = kFlutterLayerContentTypeBackingStore;
  layer.backing_store = store;

  const auto layer_bounds = DlFRect::MakeSize(frame_size_);

  const auto transformed_layer_bounds =
      root_surface_transformation_.TransformRect(layer_bounds);

  layer.offset.x = transformed_layer_bounds.x();
  layer.offset.y = transformed_layer_bounds.y();
  layer.size.width = transformed_layer_bounds.width();
  layer.size.height = transformed_layer_bounds.height();

  presented_layers_.push_back(layer);
}

static std::unique_ptr<FlutterPlatformViewMutation> ConvertMutation(
    double opacity) {
  FlutterPlatformViewMutation mutation = {};
  mutation.type = kFlutterPlatformViewMutationTypeOpacity;
  mutation.opacity = opacity;
  return std::make_unique<FlutterPlatformViewMutation>(mutation);
}

static std::unique_ptr<FlutterPlatformViewMutation> ConvertMutation(
    const DlFRect& rect) {
  FlutterPlatformViewMutation mutation = {};
  mutation.type = kFlutterPlatformViewMutationTypeClipRect;
  mutation.clip_rect.left = rect.left();
  mutation.clip_rect.top = rect.top();
  mutation.clip_rect.right = rect.right();
  mutation.clip_rect.bottom = rect.bottom();
  return std::make_unique<FlutterPlatformViewMutation>(mutation);
}

static FlutterSize VectorToSize(const DlFVector& vector) {
  FlutterSize size = {};
  size.width = vector.x();
  size.height = vector.y();
  return size;
}

static std::unique_ptr<FlutterPlatformViewMutation> ConvertMutation(
    const DlFRRect& rrect) {
  FlutterPlatformViewMutation mutation = {};
  mutation.type = kFlutterPlatformViewMutationTypeClipRoundedRect;
  const auto& rect = rrect.rect();
  mutation.clip_rounded_rect.rect.left = rect.left();
  mutation.clip_rounded_rect.rect.top = rect.top();
  mutation.clip_rounded_rect.rect.right = rect.right();
  mutation.clip_rounded_rect.rect.bottom = rect.bottom();
  mutation.clip_rounded_rect.upper_left_corner_radius =
      VectorToSize(rrect.upper_left_radii());
  mutation.clip_rounded_rect.upper_right_corner_radius =
      VectorToSize(rrect.upper_right_radii());
  mutation.clip_rounded_rect.lower_right_corner_radius =
      VectorToSize(rrect.lower_right_radii());
  mutation.clip_rounded_rect.lower_left_corner_radius =
      VectorToSize(rrect.lower_left_radii());
  return std::make_unique<FlutterPlatformViewMutation>(mutation);
}

static std::unique_ptr<FlutterPlatformViewMutation> ConvertMutation(
    const DlTransform& matrix) {
  FlutterPlatformViewMutation mutation = {};
  DlFVector3 x = matrix.eqnX_3();
  DlFVector3 y = matrix.eqnY_3();
  DlFVector3 w = matrix.eqnW_3();
  mutation.type = kFlutterPlatformViewMutationTypeTransformation;
  mutation.transformation.scaleX = x[0];
  mutation.transformation.skewX = x[1];
  mutation.transformation.transX = x[2];
  mutation.transformation.skewY = y[0];
  mutation.transformation.scaleY = y[1];
  mutation.transformation.transY = y[2];
  mutation.transformation.pers0 = w[0];
  mutation.transformation.pers1 = w[1];
  mutation.transformation.pers2 = w[2];
  return std::make_unique<FlutterPlatformViewMutation>(mutation);
}

void EmbedderLayers::PushPlatformViewLayer(
    FlutterPlatformViewIdentifier identifier,
    const EmbeddedViewParams& params) {
  {
    FlutterPlatformView view = {};
    view.struct_size = sizeof(FlutterPlatformView);
    view.identifier = identifier;

    const auto& mutators = params.mutatorsStack();

    std::vector<const FlutterPlatformViewMutation*> mutations_array;

    for (auto i = mutators.Bottom(); i != mutators.Top(); ++i) {
      const auto& mutator = *i;
      switch (mutator->GetType()) {
        case MutatorType::kClipRect: {
          mutations_array.push_back(
              mutations_referenced_
                  .emplace_back(ConvertMutation(mutator->GetRect()))
                  .get());
        } break;
        case MutatorType::kClipRRect: {
          mutations_array.push_back(
              mutations_referenced_
                  .emplace_back(ConvertMutation(mutator->GetRRect()))
                  .get());
        } break;
        case MutatorType::kClipPath: {
          // Unsupported mutation.
        } break;
        case MutatorType::kTransform: {
          const auto& matrix = mutator->GetMatrix();
          if (!matrix.IsIdentity()) {
            mutations_array.push_back(
                mutations_referenced_.emplace_back(ConvertMutation(matrix))
                    .get());
          }
        } break;
        case MutatorType::kOpacity: {
          const double opacity =
              std::clamp(mutator->GetAlphaFloat(), 0.0f, 1.0f);
          if (opacity < 1.0) {
            mutations_array.push_back(
                mutations_referenced_.emplace_back(ConvertMutation(opacity))
                    .get());
          }
        } break;
        case MutatorType::kBackdropFilter:
          break;
      }
    }

    if (!mutations_array.empty()) {
      // If there are going to be any mutations, they must first take into
      // account the root surface transformation.
      if (!root_surface_transformation_.IsIdentity()) {
        mutations_array.push_back(
            mutations_referenced_
                .emplace_back(ConvertMutation(root_surface_transformation_))
                .get());
      }

      auto mutations =
          std::make_unique<std::vector<const FlutterPlatformViewMutation*>>(
              mutations_array.rbegin(), mutations_array.rend());
      mutations_arrays_referenced_.emplace_back(std::move(mutations));

      view.mutations_count = mutations_array.size();
      view.mutations = mutations_arrays_referenced_.back().get()->data();
    }

    platform_views_referenced_.emplace_back(
        std::make_unique<FlutterPlatformView>(view));
  }

  FlutterLayer layer = {};

  layer.struct_size = sizeof(FlutterLayer);
  layer.type = kFlutterLayerContentTypePlatformView;
  layer.platform_view = platform_views_referenced_.back().get();

  const auto layer_bounds =
      DlFRect::MakeXYWH(params.finalBoundingRect().x(),                     //
                        params.finalBoundingRect().y(),                     //
                        params.sizePoints().width() * device_pixel_ratio_,  //
                        params.sizePoints().height() * device_pixel_ratio_  //
      );

  const auto transformed_layer_bounds =
      root_surface_transformation_.TransformRect(layer_bounds);

  layer.offset.x = transformed_layer_bounds.x();
  layer.offset.y = transformed_layer_bounds.y();
  layer.size.width = transformed_layer_bounds.width();
  layer.size.height = transformed_layer_bounds.height();

  presented_layers_.push_back(layer);
}

void EmbedderLayers::InvokePresentCallback(
    const PresentCallback& callback) const {
  std::vector<const FlutterLayer*> presented_layers_pointers;
  presented_layers_pointers.reserve(presented_layers_.size());
  for (const auto& layer : presented_layers_) {
    presented_layers_pointers.push_back(&layer);
  }
  callback(presented_layers_pointers);
}

}  // namespace flutter
