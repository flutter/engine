// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "fml/logging.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/texture_fill_external_image_oes.frag.h"
#include "impeller/entity/tiled_texture_fill_external_image_oes.frag.h"

namespace impeller {

using TextureExternalOESPipeline =
    RenderPipelineT<TextureFillVertexShader,
                    TextureFillExternalImageOesFragmentShader>;
using TiledTextureExternalOESPipeline =
    RenderPipelineT<TiledTextureFillVertexShader,
                    TiledTextureFillExternalImageOesFragmentShader>;

class ExternalTexturePipelineProviderGLES
    : public ExternalTexturePipelineProvider {
 public:
  explicit ExternalTexturePipelineProviderGLES(
      const ContentContext& content_context);

  ~ExternalTexturePipelineProviderGLES() override;

 protected:
  std::shared_ptr<Pipeline<PipelineDescriptor>> GetTexturePipeline(
      ContentContextOptions opts) const override {
    return content_context_.GetPipeline(texture_external_oes_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>> GetTiledTexturePipeline(
      ContentContextOptions opts) const override {
    return content_context_.GetPipeline(tiled_texture_external_oes_pipelines_,
                                        opts);
  }

 private:
  const ContentContext& content_context_;
  mutable ContentContext::Variants<TextureExternalOESPipeline>
      texture_external_oes_pipelines_;
  mutable ContentContext::Variants<TiledTextureExternalOESPipeline>
      tiled_texture_external_oes_pipelines_;
};

}  // namespace impeller
