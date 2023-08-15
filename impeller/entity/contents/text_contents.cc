// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/text_contents.h"

#include <optional>
#include <type_traits>
#include <utility>

#include "impeller/core/formats.h"
#include "impeller/core/sampler_descriptor.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/tessellator/tessellator.h"
#include "impeller/typographer/glyph_atlas.h"
#include "impeller/typographer/lazy_glyph_atlas.h"

namespace impeller {

TextContents::TextContents() = default;

TextContents::~TextContents() = default;

void TextContents::AddTextFrame(const TextFrame& frame) {
  frames_.emplace_back(TextFrameInfo{.frame = frame});
}

std::shared_ptr<GlyphAtlas> TextContents::ResolveAtlas(
    GlyphAtlas::Type type,
    const std::shared_ptr<LazyGlyphAtlas>& lazy_atlas,
    std::shared_ptr<Context> context) const {
  FML_DCHECK(lazy_atlas);
  if (lazy_atlas) {
    return lazy_atlas->CreateOrGetGlyphAtlas(type, std::move(context));
  }

  return nullptr;
}

void TextContents::SetColor(Color color) {
  color_ = color;
}

Color TextContents::GetColor() const {
  return color_.WithAlpha(color_.alpha * inherited_opacity_);
}

bool TextContents::CanInheritOpacity(const Entity& entity) const {
  return frames_.size() == 1 ? !frames_[0].frame.MaybeHasOverlapping() : false;
}

void TextContents::SetInheritedOpacity(Scalar opacity) {
  inherited_opacity_ = opacity;
}

void TextContents::SetOffset(Vector2 offset) {
  offset_ = offset;
}

std::optional<Rect> TextContents::GetTextFrameBounds() const {
  std::optional<Rect> result;
  for (const TextFrameInfo& text_frame_info : frames_) {
    auto bounds = text_frame_info.frame.GetBounds();
    if (bounds.has_value()) {
      result = result.has_value() ? result->Union(bounds.value()) : bounds;
    }
  }
  return result;
}

std::optional<Rect> TextContents::GetCoverage(const Entity& entity) const {
  auto bounds = GetTextFrameBounds();
  if (!bounds.has_value()) {
    return std::nullopt;
  }
  return bounds->TransformBounds(entity.GetTransformation() *
                                 Matrix::MakeTranslation(position_));
}

void TextContents::PopulateGlyphAtlas(
    const std::shared_ptr<LazyGlyphAtlas>& lazy_glyph_atlas,
    Scalar scale) {
  for (const TextFrameInfo& text_frame_info : frames_) {
    lazy_glyph_atlas->AddTextFrame(text_frame_info.frame, scale);
  }
  scale_ = scale;
}

bool TextContents::Render(const ContentContext& renderer,
                          const Entity& entity,
                          RenderPass& pass) const {
  auto color = GetColor();
  if (color.IsTransparent()) {
    return true;
  }

  FML_DCHECK(!frames_.empty());
  auto type = frames_[0].frame.GetAtlasType();
  auto atlas =
      ResolveAtlas(type, renderer.GetLazyGlyphAtlas(), renderer.GetContext());

  if (!atlas || !atlas->IsValid()) {
    VALIDATION_LOG << "Cannot render glyphs without prepared atlas.";
    return false;
  }

  // Information shared by all glyph draw calls.
  Command cmd;
  cmd.label = "TextFrame";
  auto opts = OptionsFromPassAndEntity(pass, entity);
  opts.primitive_type = PrimitiveType::kTriangle;
  if (type == GlyphAtlas::Type::kAlphaBitmap) {
    cmd.pipeline = renderer.GetGlyphAtlasPipeline(opts);
  } else {
    cmd.pipeline = renderer.GetGlyphAtlasColorPipeline(opts);
  }
  cmd.stencil_reference = entity.GetStencilDepth();

  using VS = GlyphAtlasPipeline::VertexShader;
  using FS = GlyphAtlasPipeline::FragmentShader;

  // Common vertex uniforms for all glyphs.
  VS::FrameInfo frame_info;
  frame_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize());
  frame_info.atlas_size =
      Vector2{static_cast<Scalar>(atlas->GetTexture()->GetSize().width),
              static_cast<Scalar>(atlas->GetTexture()->GetSize().height)};
  frame_info.offset = offset_;
  frame_info.is_translation_scale =
      entity.GetTransformation().IsTranslationScaleOnly();
  frame_info.entity_transform = entity.GetTransformation();
  frame_info.text_color = ToVector(color.Premultiply());

  VS::BindFrameInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frame_info));

  SamplerDescriptor sampler_desc;
  if (frame_info.is_translation_scale) {
    sampler_desc.min_filter = MinMagFilter::kNearest;
    sampler_desc.mag_filter = MinMagFilter::kNearest;
  } else {
    // Currently, we only propagate the scale of the transform to the atlas
    // renderer, so if the transform has more than just a translation, we turn
    // on linear sampling to prevent crunchiness caused by the pixel grid not
    // being perfectly aligned.
    // The downside is that this slightly over-blurs rotated/skewed text.
    sampler_desc.min_filter = MinMagFilter::kLinear;
    sampler_desc.mag_filter = MinMagFilter::kLinear;
  }
  sampler_desc.mip_filter = MipFilter::kNearest;

  FS::BindGlyphAtlasSampler(
      cmd,                  // command
      atlas->GetTexture(),  // texture
      renderer.GetContext()->GetSamplerLibrary()->GetSampler(
          sampler_desc)  // sampler
  );

  // Common vertex information for all glyphs.
  // All glyphs are given the same vertex information in the form of a
  // unit-sized quad. The size of the glyph is specified in per instance data
  // and the vertex shader uses this to size the glyph correctly. The
  // interpolated vertex information is also used in the fragment shader to
  // sample from the glyph atlas.

  constexpr std::array<Point, 6> unit_points = {Point{0, 0}, Point{1, 0},
                                                Point{0, 1}, Point{1, 0},
                                                Point{0, 1}, Point{1, 1}};

  auto& host_buffer = pass.GetTransientsBuffer();
  size_t vertex_count = 0;
  for (const TextFrameInfo& frame_info : frames_) {
    for (const auto& run : frame_info.frame.GetRuns()) {
      vertex_count += run.GetGlyphPositions().size();
    }
  }
  vertex_count *= 6;

  auto buffer_view = host_buffer.Emplace(
      vertex_count * sizeof(VS::PerVertexData), alignof(VS::PerVertexData),
      [&](uint8_t* contents) {
        VS::PerVertexData vtx;
        size_t vertex_offset = 0;
        for (const TextFrameInfo& frame_info : frames_) {
          for (const auto& run : frame_info.frame.GetRuns()) {
            const Font& font = run.GetFont();
            auto rounded_scale = TextFrame::RoundScaledFontSize(
                scale_, font.GetMetrics().point_size);

            for (const auto& glyph_position : run.GetGlyphPositions()) {
              FontGlyphPair font_glyph_pair{font, glyph_position.glyph,
                                            rounded_scale};
              auto maybe_atlas_glyph_bounds =
                  atlas->FindFontGlyphBounds(font_glyph_pair);
              if (!maybe_atlas_glyph_bounds.has_value()) {
                VALIDATION_LOG << "Could not find glyph position in the atlas.";
                continue;
              }
              auto atlas_glyph_bounds = maybe_atlas_glyph_bounds.value();
              vtx.atlas_glyph_bounds = Vector4(atlas_glyph_bounds.origin.x,
                                               atlas_glyph_bounds.origin.y,
                                               atlas_glyph_bounds.size.width,
                                               atlas_glyph_bounds.size.height);
              vtx.glyph_bounds =
                  Vector4(glyph_position.glyph.bounds.origin.x,
                          glyph_position.glyph.bounds.origin.y,
                          glyph_position.glyph.bounds.size.width,
                          glyph_position.glyph.bounds.size.height);
              vtx.glyph_position = position_ + glyph_position.position;

              for (const auto& point : unit_points) {
                vtx.unit_position = point;
                ::memcpy(contents + vertex_offset, &vtx,
                         sizeof(VS::PerVertexData));
                vertex_offset += sizeof(VS::PerVertexData);
              }
            }
          }
        }
      });

  cmd.BindVertices({
      .vertex_buffer = buffer_view,
      .index_buffer = {},
      .vertex_count = vertex_count,
      .index_type = IndexType::kNone,
  });

  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
