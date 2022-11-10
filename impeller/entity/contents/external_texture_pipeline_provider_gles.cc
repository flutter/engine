// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/external_texture_pipeline_provider_gles.h"

namespace impeller {

ExternalTexturePipelineProviderGLES::ExternalTexturePipelineProviderGLES(
    const ContentContext& content_context)
    : content_context_(content_context) {
  texture_external_oes_pipelines_[{}] =
      content_context_.CreateDefaultPipeline<TextureExternalOESPipeline>(
          *content_context_.GetContext());
  tiled_texture_external_oes_pipelines_[{}] =
      content_context_.CreateDefaultPipeline<TiledTextureExternalOESPipeline>(
          *content_context_.GetContext());
}

ExternalTexturePipelineProviderGLES::~ExternalTexturePipelineProviderGLES() =
    default;

}  // namespace impeller
