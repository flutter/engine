// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

#include "flutter/fml/hash_combine.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"
#include "impeller/base/validation.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/pipeline.h"

#include "impeller/entity/framebuffer_blend.vert.h"
#include "impeller/entity/framebuffer_blend_color.frag.h"
#include "impeller/entity/framebuffer_blend_colorburn.frag.h"
#include "impeller/entity/framebuffer_blend_colordodge.frag.h"
#include "impeller/entity/framebuffer_blend_darken.frag.h"
#include "impeller/entity/framebuffer_blend_difference.frag.h"
#include "impeller/entity/framebuffer_blend_exclusion.frag.h"
#include "impeller/entity/framebuffer_blend_hardlight.frag.h"
#include "impeller/entity/framebuffer_blend_hue.frag.h"
#include "impeller/entity/framebuffer_blend_lighten.frag.h"
#include "impeller/entity/framebuffer_blend_luminosity.frag.h"
#include "impeller/entity/framebuffer_blend_multiply.frag.h"
#include "impeller/entity/framebuffer_blend_overlay.frag.h"
#include "impeller/entity/framebuffer_blend_saturation.frag.h"
#include "impeller/entity/framebuffer_blend_screen.frag.h"
#include "impeller/entity/framebuffer_blend_softlight.frag.h"

namespace impeller {

using FramebufferBlendColorPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendColorFragmentShader>;
using FramebufferBlendColorBurnPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendColorburnFragmentShader>;
using FramebufferBlendColorDodgePipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendColordodgeFragmentShader>;
using FramebufferBlendDarkenPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendDarkenFragmentShader>;
using FramebufferBlendDifferencePipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendDifferenceFragmentShader>;
using FramebufferBlendExclusionPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendExclusionFragmentShader>;
using FramebufferBlendHardLightPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendHardlightFragmentShader>;
using FramebufferBlendHuePipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendHueFragmentShader>;
using FramebufferBlendLightenPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendLightenFragmentShader>;
using FramebufferBlendLuminosityPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendLuminosityFragmentShader>;
using FramebufferBlendMultiplyPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendMultiplyFragmentShader>;
using FramebufferBlendOverlayPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendOverlayFragmentShader>;
using FramebufferBlendSaturationPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendSaturationFragmentShader>;
using FramebufferBlendScreenPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendScreenFragmentShader>;
using FramebufferBlendSoftLightPipeline =
    RenderPipelineT<FramebufferBlendVertexShader,
                    FramebufferBlendSoftlightFragmentShader>;

class FramebufferBlendContext {
 public:
  explicit FramebufferBlendContext(std::shared_ptr<Context> context);

  ~FramebufferBlendContext();

  bool IsValid() const;

  using FramebufferBlendColorPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendColorFragmentShader>;
  using FramebufferBlendColorBurnPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendColorburnFragmentShader>;
  using FramebufferBlendColorDodgePipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendColordodgeFragmentShader>;
  using FramebufferBlendDarkenPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendDarkenFragmentShader>;
  using FramebufferBlendDifferencePipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendDifferenceFragmentShader>;
  using FramebufferBlendExclusionPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendExclusionFragmentShader>;
  using FramebufferBlendHardLightPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendHardlightFragmentShader>;
  using FramebufferBlendHuePipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendHueFragmentShader>;
  using FramebufferBlendLightenPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendLightenFragmentShader>;
  using FramebufferBlendLuminosityPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendLuminosityFragmentShader>;
  using FramebufferBlendMultiplyPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendMultiplyFragmentShader>;
  using FramebufferBlendOverlayPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendOverlayFragmentShader>;
  using FramebufferBlendSaturationPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendSaturationFragmentShader>;
  using FramebufferBlendScreenPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendScreenFragmentShader>;
  using FramebufferBlendSoftLightPipeline =
      RenderPipelineT<FramebufferBlendVertexShader,
                      FramebufferBlendSoftlightFragmentShader>;

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendColorPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_color_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendColorBurnPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_colorburn_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendColorDodgePipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_colordodge_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendDarkenPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_darken_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendDifferencePipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_difference_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendExclusionPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_exclusion_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendHardLightPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_hardlight_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>> GetFramebufferBlendHuePipeline(
      ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_hue_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendLightenPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_lighten_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendLuminosityPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_luminosity_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendMultiplyPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_multiply_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendOverlayPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_overlay_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendSaturationPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_saturation_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendScreenPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_screen_pipelines_, opts);
  }

  std::shared_ptr<Pipeline<PipelineDescriptor>>
  GetFramebufferBlendSoftLightPipeline(ContentContextOptions opts) const {
    return GetPipeline(framebuffer_blend_softlight_pipelines_, opts);
  }

 private:
  std::shared_ptr<Context> context_;

  template <class T>
  using Variants = std::unordered_map<ContentContextOptions,
                                      std::unique_ptr<T>,
                                      ContentContextOptions::Hash,
                                      ContentContextOptions::Equal>;

  // These are mutable because while the prototypes are created eagerly, any
  // variants requested from that are lazily created and cached in the variants
  // map.
  mutable Variants<FramebufferBlendColorPipeline>
      framebuffer_blend_color_pipelines_;
  mutable Variants<FramebufferBlendColorBurnPipeline>
      framebuffer_blend_colorburn_pipelines_;
  mutable Variants<FramebufferBlendColorDodgePipeline>
      framebuffer_blend_colordodge_pipelines_;
  mutable Variants<FramebufferBlendDarkenPipeline>
      framebuffer_blend_darken_pipelines_;
  mutable Variants<FramebufferBlendDifferencePipeline>
      framebuffer_blend_difference_pipelines_;
  mutable Variants<FramebufferBlendExclusionPipeline>
      framebuffer_blend_exclusion_pipelines_;
  mutable Variants<FramebufferBlendHardLightPipeline>
      framebuffer_blend_hardlight_pipelines_;
  mutable Variants<FramebufferBlendHuePipeline>
      framebuffer_blend_hue_pipelines_;
  mutable Variants<FramebufferBlendLightenPipeline>
      framebuffer_blend_lighten_pipelines_;
  mutable Variants<FramebufferBlendLuminosityPipeline>
      framebuffer_blend_luminosity_pipelines_;
  mutable Variants<FramebufferBlendMultiplyPipeline>
      framebuffer_blend_multiply_pipelines_;
  mutable Variants<FramebufferBlendOverlayPipeline>
      framebuffer_blend_overlay_pipelines_;
  mutable Variants<FramebufferBlendSaturationPipeline>
      framebuffer_blend_saturation_pipelines_;
  mutable Variants<FramebufferBlendScreenPipeline>
      framebuffer_blend_screen_pipelines_;
  mutable Variants<FramebufferBlendSoftLightPipeline>
      framebuffer_blend_softlight_pipelines_;

  template <class TypedPipeline>
  std::shared_ptr<Pipeline<PipelineDescriptor>> GetPipeline(
      Variants<TypedPipeline>& container,
      ContentContextOptions opts) const {
    if (!IsValid()) {
      return nullptr;
    }

    if (wireframe_) {
      opts.wireframe = true;
    }

    if (auto found = container.find(opts); found != container.end()) {
      return found->second->WaitAndGet();
    }

    auto prototype = container.find({});

    // The prototype must always be initialized in the constructor.
    FML_CHECK(prototype != container.end());

    auto variant_future = prototype->second->WaitAndGet()->CreateVariant(
        [&opts, variants_count = container.size()](PipelineDescriptor& desc) {
          opts.ApplyToPipelineDescriptor(desc);
          desc.SetLabel(
              SPrintF("%s V#%zu", desc.GetLabel().c_str(), variants_count));
        });
    auto variant = std::make_unique<TypedPipeline>(std::move(variant_future));
    auto variant_pipeline = variant->WaitAndGet();
    container[opts] = std::move(variant);
    return variant_pipeline;
  }

  bool is_valid_ = false;
  bool wireframe_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(FramebufferBlendContext);
};

}  // namespace impeller
