// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/color.h"
#include "impeller/renderer/context.h"
#include "impeller/renderer/pipeline_descriptor.h"

namespace impeller {

/// Pipeline state configuration.
///
/// Each unique combination of these options requires a different pipeline state
/// object to be built. This struct is used as a key for the per-pipeline
/// variant cache.
///
/// When adding fields to this key, reliant features should take care to limit
/// the combinatorical explosion of variations. A sufficiently complicated
/// Flutter application may easily require building hundreds of PSOs in total,
/// but they shouldn't require e.g. 10s of thousands.
struct ContentContextOptions {
  SampleCount sample_count = SampleCount::kCount1;
  BlendMode blend_mode = BlendMode::kSourceOver;
  CompareFunction stencil_compare = CompareFunction::kEqual;
  StencilOperation stencil_operation = StencilOperation::kKeep;
  PrimitiveType primitive_type = PrimitiveType::kTriangle;
  std::optional<PixelFormat> color_attachment_pixel_format;
  bool has_stencil_attachment = true;
  bool wireframe = false;

  struct Hash {
    constexpr std::size_t operator()(const ContentContextOptions& o) const {
      return fml::HashCombine(o.sample_count, o.blend_mode, o.stencil_compare,
                              o.stencil_operation, o.primitive_type,
                              o.color_attachment_pixel_format,
                              o.has_stencil_attachment, o.wireframe);
    }
  };

  struct Equal {
    constexpr bool operator()(const ContentContextOptions& lhs,
                              const ContentContextOptions& rhs) const {
      return lhs.sample_count == rhs.sample_count &&
             lhs.blend_mode == rhs.blend_mode &&
             lhs.stencil_compare == rhs.stencil_compare &&
             lhs.stencil_operation == rhs.stencil_operation &&
             lhs.primitive_type == rhs.primitive_type &&
             lhs.color_attachment_pixel_format ==
                 rhs.color_attachment_pixel_format &&
             lhs.has_stencil_attachment == rhs.has_stencil_attachment &&
             lhs.wireframe == rhs.wireframe;
    }
  };

  void ApplyToPipelineDescriptor(PipelineDescriptor& desc) const;
};

constexpr std::array<BlendMode, 14> kColorSourceBlends = {
    BlendMode::kClear,
    BlendMode::kSource,
    BlendMode::kDestination,
    BlendMode::kSourceOver,
    BlendMode::kDestinationOver,
    BlendMode::kSourceIn,
    BlendMode::kDestinationIn,
    BlendMode::kSourceOut,
    BlendMode::kDestinationOut,
    BlendMode::kSourceATop,
    BlendMode::kDestinationATop,
    BlendMode::kXor,
    BlendMode::kPlus,
    BlendMode::kModulate,
};

struct StencilParameters {
  CompareFunction stencil_compare = CompareFunction::kEqual;
  StencilOperation stencil_operation = StencilOperation::kKeep;
};

constexpr std::array<StencilParameters, 2> kColorSourceStencilParams = {
    StencilParameters{.stencil_compare = CompareFunction::kEqual,
                      .stencil_operation = StencilOperation::kKeep},
    StencilParameters{
        .stencil_compare = CompareFunction::kEqual,
        .stencil_operation = StencilOperation::kIncrementClamp,
    }};

constexpr std::array<PrimitiveType, 2> kColorSourcePrimitiveTypes = {
    PrimitiveType::kTriangle, PrimitiveType::kTriangleStrip};

template <class T>
using Variants = std::unordered_map<ContentContextOptions,
                                    std::unique_ptr<T>,
                                    ContentContextOptions::Hash,
                                    ContentContextOptions::Equal>;

template <typename PipelineT>
static std::unique_ptr<PipelineT> CreatePipeline(
    const Context& context,
    ContentContextOptions options) {
  auto desc = PipelineT::Builder::MakeDefaultPipelineDescriptor(context);
  if (!desc.has_value()) {
    return nullptr;
  }
  options.ApplyToPipelineDescriptor(*desc);
  return std::make_unique<PipelineT>(context, desc);
}

/// @brief Initialize all possible pipeline variants for the given color source
/// pipeline.
template <typename PipelineT>
void InitializeColorSource(Variants<PipelineT>& storage,
                           ContentContextOptions& default_opts,
                           const Context& context) {
  if (context.GetCapabilities()->HasSlowPSOConstruction()) {
    for (const auto& blend : kColorSourceBlends) {
      for (const auto& stencil_config : kColorSourceStencilParams) {
        for (const auto& primitive_type : kColorSourcePrimitiveTypes) {
          ContentContextOptions options = default_opts;
          options.blend_mode = blend;
          options.stencil_compare = stencil_config.stencil_compare;
          options.stencil_operation = stencil_config.stencil_operation;
          options.primitive_type = primitive_type;
          storage[options] = CreatePipeline<PipelineT>(context, options);
        }
      }
    }
  } else {
    storage[default_opts] = CreatePipeline<PipelineT>(context, default_opts);
  }
}

}  // namespace impeller
