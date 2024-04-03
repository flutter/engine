// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_ENTITY_CANVAS_H_
#define FLUTTER_IMPELLER_ENTITY_ENTITY_CANVAS_H_

#include <deque>
#include <memory>
#include <optional>
#include <vector>

#include "impeller/entity/contents/color_source_contents.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/entity_pass.h"
#include "impeller/entity/inline_pass_context.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/matrix.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

class EntityCanvas {
 public:
  EntityCanvas(ContentContext& renderer, RenderTarget& render_target);

  ~EntityCanvas();

  const Matrix& GetCurrentTransform() const;

  const std::optional<Rect> GetCurrentLocalCullingBounds() const;

  size_t GetClipDepth() const;

  size_t GetSaveCount() const;

  void RestoreToCount(size_t count);

  // Transforms
  void ResetTransform();

  void Transform(const Matrix& transform);

  void Concat(const Matrix& transform);

  void PreConcat(const Matrix& transform);

  void Translate(const Vector3& offset);

  void Scale(const Vector2& scale);

  void Scale(const Vector3& scale);

  void Skew(Scalar sx, Scalar sy);

  void Rotate(Radians radians);

  // Save/Restore
  void Save();

  void SaveLayer(
      Rect bounds,
      BlendMode blend_mode,
      Scalar opacity,
      ContentBoundsPromise bounds_promise = ContentBoundsPromise::kUnknown);

  bool Restore();

  // Drawing.
  void DrawRect(const Rect& rect, Color& color, BlendMode blend_mode);

  void DrawPaint(Color& color, BlendMode blend_mode);

  void DrawCircle(const Point& center,
                  Scalar radius,
                  Color& color,
                  BlendMode blend_mode);

  void DrawPath(const Path& path, Color& color, BlendMode blend_mode);

  void DrawImage(const std::shared_ptr<Texture>& image,
                 Point offset,
                 BlendMode blend_mode,
                 SamplerDescriptor sampler);

  void DrawImageRect(const std::shared_ptr<Texture>& image,
                     Rect source,
                     Rect dest,
                     BlendMode blend_mode,
                     SamplerDescriptor sampler,
                     bool strict_src_rect);

  void DrawTextFrame(const std::shared_ptr<TextFrame>& text_frame,
                     Point position,
                     Color color,
                     BlendMode blend_mode);

  void DrawLine(const Point& p0,
                const Point& p1,
                Color color,
                BlendMode blend_mode,
                Scalar width,
                Cap cap);

  void DrawRRect(const Rect& rect,
                 const Size& corner_radii,
                 Color color,
                 BlendMode blend_mode);

  void EndReplay() {
    FML_DCHECK(inline_pass_contexts_.size() == 1u);
    inline_pass_contexts_.back()->EndPass();

    render_passes_.clear();
    inline_pass_contexts_.clear();

    renderer_.GetRenderTargetCache()->End();
  }

 private:
  void Draw(const ColorSourceContents& color_source, BlendMode blend_mode);

  struct CanvasStackEntry {
    Matrix transform;
    // |cull_rect| is conservative screen-space bounds of the clipped output
    // area
    std::optional<Rect> cull_rect;
    size_t clip_depth = 0u;
    // The number of clips tracked for this canvas stack entry.
    size_t num_clips = 0u;
    Entity::RenderingMode rendering_mode = Entity::RenderingMode::kDirect;
  };

  struct SaveLayerState {
    BlendMode blend_mode;
    Scalar opacity;
    Rect coverage;
  };

  Point GetGlobalPassPosition() {
    if (save_layer_state_.empty()) {
      return Point(0, 0);
    }
    return save_layer_state_.back().coverage.GetOrigin();
  }

  ContentContext& renderer_;
  std::deque<CanvasStackEntry> transform_stack_;
  std::optional<Rect> initial_cull_rect_;
  std::vector<std::unique_ptr<InlinePassContext>> inline_pass_contexts_;
  std::vector<std::unique_ptr<EntityPassTarget>> entity_pass_targets_;
  std::vector<SaveLayerState> save_layer_state_;
  std::vector<std::shared_ptr<RenderPass>> render_passes_;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_ENTITY_CANVAS_H_
