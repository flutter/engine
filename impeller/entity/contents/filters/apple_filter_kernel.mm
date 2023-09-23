// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/apple_filter_kernel.h"

#include <MetalPerformanceShaders/MetalPerformanceShaders.h>
#include <optional>

#include "impeller/core/formats.h"
#include "impeller/entity/contents/anonymous_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/entity/contents/filters/inputs/filter_input.h"
#include "impeller/entity/contents/solid_color_contents.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/renderer/backend/metal/context_mtl.h"  // nogncheck
#include "impeller/renderer/backend/metal/texture_mtl.h"  // nogncheck
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/renderer/snapshot.h"

namespace impeller {

AppleGaussianBlurFilterContents::AppleGaussianBlurFilterContents() {}

AppleGaussianBlurFilterContents::~AppleGaussianBlurFilterContents() {}

void AppleGaussianBlurFilterContents::SetSigma(Sigma sigma_x, Sigma sigma_y) {
  sigma_x_ = sigma_x;
  sigma_y_ = sigma_y;
}

void AppleGaussianBlurFilterContents::SetTileMode(Entity::TileMode tile_mode) {
  tile_mode_ = tile_mode;
}

// |FilterContents|
std::optional<Entity> AppleGaussianBlurFilterContents::RenderFilter(
    const FilterInput::Vector& inputs,
    const ContentContext& renderer,
    const Entity& entity,
    const Matrix& effect_transform,
    const Rect& coverage,
    const std::optional<Rect>& coverage_hint) const {
  if (inputs.size() == 0) {
    return std::nullopt;
  }
  auto input_snapshot = inputs[0]->GetSnapshot("GaussianBlurInput", renderer,
                                               entity, coverage_hint);
  if (!input_snapshot.has_value()) {
    return std::nullopt;
  }

  auto* context = ContextMTL::Cast(renderer.GetContext().get());
  MPSImageGaussianBlur* kernel =
      [[MPSImageGaussianBlur alloc] initWithDevice:context->GetDevice()
                                             sigma:sigma_x_.sigma];

  TextureDescriptor descriptor;
  descriptor.size = ISize::Ceil(coverage.size);
  descriptor.format = input_snapshot->texture->GetTextureDescriptor().format;
  descriptor.usage = static_cast<uint64_t>(TextureUsage::kShaderWrite) |
                     static_cast<uint64_t>(TextureUsage::kShaderRead);
  descriptor.sample_count = SampleCount::kCount1;
  descriptor.storage_mode = StorageMode::kDevicePrivate;

  auto destination =
      renderer.GetContext()->GetResourceAllocator()->CreateTexture(descriptor);
  destination->SetLabel("GaussianOutput");

  id<MTLTexture> input_texture =
      std::static_pointer_cast<TextureMTL>(input_snapshot->texture)
          ->GetMTLTexture();
  id<MTLTexture> output_texture =
      std::static_pointer_cast<TextureMTL>(destination)->GetMTLTexture();

  id<MTLCommandBuffer> command_buffer =
      context->CreateMTLCommandBuffer("AppleBlur");

  [kernel encodeToCommandBuffer:command_buffer
                  sourceTexture:input_texture
             destinationTexture:output_texture];
  [command_buffer commit];

  return Entity::FromSnapshot(
      Snapshot{.texture = destination, .transform = input_snapshot->transform},
      entity.GetBlendMode(), entity.GetStencilDepth());
}

}  // namespace impeller