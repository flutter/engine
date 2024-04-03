// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/entity_canvas.h"

#include "impeller/entity/contents/solid_color_contents.h"
#include "impeller/entity/contents/text_contents.h"
#include "impeller/entity/contents/texture_contents.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/entity_pass_target.h"
#include "impeller/entity/geometry/circle_geometry.h"
#include "impeller/entity/geometry/cover_geometry.h"
#include "impeller/entity/geometry/fill_path_geometry.h"
#include "impeller/entity/geometry/line_geometry.h"
#include "impeller/entity/geometry/rect_geometry.h"
#include "impeller/entity/geometry/round_rect_geometry.h"
#include "impeller/geometry/color.h"

namespace impeller {

static const constexpr RenderTarget::AttachmentConfig kDefaultStencilConfig =
    RenderTarget::AttachmentConfig{
        .storage_mode = StorageMode::kDeviceTransient,
        .load_action = LoadAction::kDontCare,
        .store_action = StoreAction::kDontCare,
    };

static std::unique_ptr<EntityPassTarget> CreateRenderTarget(
    ContentContext& renderer,
    ISize size,
    int mip_count,
    const Color& clear_color) {
  const std::shared_ptr<Context>& context = renderer.GetContext();

  /// All of the load/store actions are managed by `InlinePassContext` when
  /// `RenderPasses` are created, so we just set them to `kDontCare` here.
  /// What's important is the `StorageMode` of the textures, which cannot be
  /// changed for the lifetime of the textures.

  if (context->GetBackendType() == Context::BackendType::kOpenGLES) {
    // TODO(https://github.com/flutter/flutter/issues/141732): Implement mip map
    // generation on opengles.
    mip_count = 1;
  }

  RenderTarget target;
  if (context->GetCapabilities()->SupportsOffscreenMSAA()) {
    target = renderer.GetRenderTargetCache()->CreateOffscreenMSAA(
        /*context=*/*context,
        /*size=*/size,
        /*mip_count=*/mip_count,
        /*label=*/"EntityPass",
        /*color_attachment_config=*/
        RenderTarget::AttachmentConfigMSAA{
            .storage_mode = StorageMode::kDeviceTransient,
            .resolve_storage_mode = StorageMode::kDevicePrivate,
            .load_action = LoadAction::kDontCare,
            .store_action = StoreAction::kMultisampleResolve,
            .clear_color = clear_color},
        /*stencil_attachment_config=*/
        kDefaultStencilConfig);
  } else {
    target = renderer.GetRenderTargetCache()->CreateOffscreen(
        *context,  // context
        size,      // size
        /*mip_count=*/mip_count,
        "EntityPass",  // label
        RenderTarget::AttachmentConfig{
            .storage_mode = StorageMode::kDevicePrivate,
            .load_action = LoadAction::kDontCare,
            .store_action = StoreAction::kDontCare,
            .clear_color = clear_color,
        },                     // color_attachment_config
        kDefaultStencilConfig  // stencil_attachment_config
    );
  }

  return std::make_unique<EntityPassTarget>(
      target, renderer.GetDeviceCapabilities().SupportsReadFromResolve(),
      renderer.GetDeviceCapabilities().SupportsImplicitResolvingMSAA());
}

EntityCanvas::EntityCanvas(ContentContext& renderer,
                           RenderTarget& render_target)
    : renderer_(renderer) {
  initial_cull_rect_ = Rect::MakeSize(render_target.GetRenderTargetSize());
  transform_stack_.emplace_back(
      CanvasStackEntry{.cull_rect = initial_cull_rect_});

  auto color0 = render_target.GetColorAttachments().find(0u)->second;

  auto& stencil_attachment = render_target.GetStencilAttachment();
  auto& depth_attachment = render_target.GetDepthAttachment();
  if (!stencil_attachment.has_value() || !depth_attachment.has_value()) {
    // Setup a new root stencil with an optimal configuration if one wasn't
    // provided by the caller.
    render_target.SetupDepthStencilAttachments(
        *renderer.GetContext(), *renderer.GetContext()->GetResourceAllocator(),
        color0.texture->GetSize(),
        renderer.GetContext()->GetCapabilities()->SupportsOffscreenMSAA(),
        "ImpellerOnscreen", kDefaultStencilConfig);
  }

  // Set up the clear color of the root pass.
  color0.clear_color = Color::BlackTransparent();
  render_target.SetColorAttachment(color0, 0);

  entity_pass_targets_.push_back(std::make_unique<EntityPassTarget>(
      render_target, renderer.GetDeviceCapabilities().SupportsReadFromResolve(),
      renderer.GetDeviceCapabilities().SupportsImplicitResolvingMSAA()));

  auto inline_pass = std::make_unique<InlinePassContext>(
      renderer_, *entity_pass_targets_.back(), 0, 0);
  inline_pass_contexts_.emplace_back(std::move(inline_pass));
  auto result = inline_pass_contexts_.back()->GetRenderPass(0u);
  render_passes_.push_back(result.pass);

  renderer.GetRenderTargetCache()->Start();
}

EntityCanvas::~EntityCanvas() = default;

size_t EntityCanvas::GetClipDepth() const {
  return transform_stack_.back().clip_depth;
}

const Matrix& EntityCanvas::GetCurrentTransform() const {
  return transform_stack_.back().transform;
}

size_t EntityCanvas::GetSaveCount() const {
  return transform_stack_.size();
}

void EntityCanvas::RestoreToCount(size_t count) {
  while (GetSaveCount() > count) {
    if (!Restore()) {
      return;
    }
  }
}

const std::optional<Rect> EntityCanvas::GetCurrentLocalCullingBounds() const {
  auto cull_rect = transform_stack_.back().cull_rect;
  if (cull_rect.has_value()) {
    Matrix inverse = transform_stack_.back().transform.Invert();
    cull_rect = cull_rect.value().TransformBounds(inverse);
  }
  return cull_rect;
}

// Transform Methods.

void EntityCanvas::ResetTransform() {
  transform_stack_.back().transform = {};
}

void EntityCanvas::Transform(const Matrix& transform) {
  Concat(transform);
}

void EntityCanvas::Concat(const Matrix& transform) {
  transform_stack_.back().transform = GetCurrentTransform() * transform;
}

void EntityCanvas::PreConcat(const Matrix& transform) {
  transform_stack_.back().transform = transform * GetCurrentTransform();
}

void EntityCanvas::Translate(const Vector3& offset) {
  Concat(Matrix::MakeTranslation(offset));
}

void EntityCanvas::Scale(const Vector2& scale) {
  Concat(Matrix::MakeScale(scale));
}

void EntityCanvas::Scale(const Vector3& scale) {
  Concat(Matrix::MakeScale(scale));
}

void EntityCanvas::Skew(Scalar sx, Scalar sy) {
  Concat(Matrix::MakeSkew(sx, sy));
}

void EntityCanvas::Rotate(Radians radians) {
  Concat(Matrix::MakeRotationZ(radians));
}

// Save Methods

void EntityCanvas::Save() {
  auto entry = CanvasStackEntry{};
  entry.transform = transform_stack_.back().transform;
  entry.cull_rect = transform_stack_.back().cull_rect;
  entry.clip_depth = transform_stack_.back().clip_depth;
  entry.rendering_mode = Entity::RenderingMode::kDirect;
  transform_stack_.emplace_back(entry);
}

void EntityCanvas::SaveLayer(Rect bounds,
                             BlendMode blend_mode,
                             Scalar opacity,
                             ContentBoundsPromise bounds_promise) {
  Rect subpass_coverage = bounds.TransformBounds(GetCurrentTransform());
  auto target =
      CreateRenderTarget(renderer_,
                         ISize::MakeWH(subpass_coverage.GetSize().width,
                                       subpass_coverage.GetSize().height),
                         1u, Color::BlackTransparent());
  entity_pass_targets_.push_back(std::move(target));
  save_layer_state_.push_back(
      SaveLayerState{blend_mode, opacity, subpass_coverage});

  CanvasStackEntry entry;
  entry.transform = transform_stack_.back().transform;
  entry.cull_rect = transform_stack_.back().cull_rect;
  entry.clip_depth = transform_stack_.back().clip_depth;
  entry.rendering_mode = Entity::RenderingMode::kSubpass;
  transform_stack_.emplace_back(entry);

  auto inline_pass = std::make_unique<InlinePassContext>(
      renderer_, *entity_pass_targets_.back(), 0, 0);
  inline_pass_contexts_.emplace_back(std::move(inline_pass));

  auto result = inline_pass_contexts_.back()->GetRenderPass(0u);
  render_passes_.push_back(result.pass);
}

bool EntityCanvas::Restore() {
  FML_DCHECK(transform_stack_.size() > 0);
  if (transform_stack_.size() == 1) {
    return false;
  }

  if (transform_stack_.back().rendering_mode ==
      Entity::RenderingMode::kSubpass) {
    auto inline_pass = std::move(inline_pass_contexts_.back());

    SaveLayerState save_layer_state = save_layer_state_.back();
    save_layer_state_.pop_back();

    auto contents = TextureContents::MakeRect(Rect::MakeSize(
        inline_pass->GetPassTarget().GetRenderTarget().GetRenderTargetSize()));
    contents->SetTexture(inline_pass->GetTexture());
    contents->SetLabel("Subpass");
    contents->SetSourceRect(Rect::MakeSize(
        inline_pass->GetPassTarget().GetRenderTarget().GetRenderTargetSize()));
    contents->SetOpacity(save_layer_state.opacity);
    contents->SetDeferApplyingOpacity(false);

    inline_pass->EndPass();
    render_passes_.pop_back();
    inline_pass_contexts_.pop_back();

    Entity element_entity;
    element_entity.SetClipDepth(GetClipDepth());
    element_entity.SetContents(std::move(contents));
    element_entity.SetBlendMode(save_layer_state.blend_mode);
    element_entity.SetTransform(Matrix::MakeTranslation(
        Vector3(save_layer_state.coverage.GetOrigin())));
    element_entity.Render(renderer_, *render_passes_.back());
  }

  transform_stack_.pop_back();

  return true;
}

// Drawing.
void EntityCanvas::DrawRect(const Rect& rect,
                            Color& color,
                            BlendMode blend_mode) {
  auto geom = RectGeometry(rect);
  SolidColorContents color_source;
  color_source.SetRawGeometry(&geom);
  color_source.SetColor(color);

  Draw(color_source, blend_mode);
}

void EntityCanvas::DrawPaint(Color& color, BlendMode blend_mode) {
  auto geom = CoverGeometry();
  SolidColorContents color_source;
  color_source.SetRawGeometry(&geom);
  color_source.SetColor(color);

  Draw(color_source, blend_mode);
}

void EntityCanvas::DrawCircle(const Point& center,
                              Scalar radius,
                              Color& color,
                              BlendMode blend_mode) {
  CircleGeometry geom(center, radius);
  SolidColorContents color_source;
  color_source.SetRawGeometry(&geom);
  color_source.SetColor(color);

  Draw(color_source, blend_mode);
}

void EntityCanvas::DrawPath(const Path& path,
                            Color& color,
                            BlendMode blend_mode) {
  FillPathGeometry geom(path);
  SolidColorContents color_source;
  color_source.SetRawGeometry(&geom);
  color_source.SetColor(color);

  Draw(color_source, blend_mode);
}

void EntityCanvas::DrawImage(const std::shared_ptr<Texture>& image,
                             Point offset,
                             BlendMode blend_mode,
                             SamplerDescriptor sampler) {
  if (!image) {
    return;
  }

  const auto source = Rect::MakeSize(image->GetSize());
  const auto dest = source.Shift(offset);

  DrawImageRect(image, source, dest, blend_mode, std::move(sampler), false);
}

void EntityCanvas::DrawImageRect(const std::shared_ptr<Texture>& image,
                                 Rect source,
                                 Rect dest,
                                 BlendMode blend_mode,
                                 SamplerDescriptor sampler,
                                 bool strict_src_rect) {
  if (!image || source.IsEmpty() || dest.IsEmpty()) {
    return;
  }

  auto size = image->GetSize();

  if (size.IsEmpty()) {
    return;
  }

  TextureContents texture_contents;
  texture_contents.SetDestinationRect(dest);
  texture_contents.SetTexture(image);
  texture_contents.SetSourceRect(source);
  texture_contents.SetStrictSourceRect(strict_src_rect);
  texture_contents.SetSamplerDescriptor(std::move(sampler));
  texture_contents.SetOpacity(1.0);
  texture_contents.SetDeferApplyingOpacity(false);

  Entity entity;
  entity.SetBlendMode(blend_mode);
  entity.SetClipDepth(GetClipDepth());
  entity.SetTransform(
      Matrix::MakeTranslation(Vector3(-GetGlobalPassPosition())) *
      GetCurrentTransform());

  texture_contents.Render(renderer_, entity, *render_passes_.back());
}

void EntityCanvas::DrawTextFrame(const std::shared_ptr<TextFrame>& text_frame,
                                 Point position,
                                 Color color,
                                 BlendMode blend_mode) {
  Entity entity;
  entity.SetClipDepth(GetClipDepth());
  entity.SetBlendMode(blend_mode);

  TextContents text_contents;
  text_contents.SetTextFrame(text_frame);
  text_contents.SetColor(color);
  text_contents.SetForceTextColor(false);
  text_contents.SetScale(2.625);

  entity.SetTransform(
      Matrix::MakeTranslation(Vector3(-GetGlobalPassPosition())) *
      GetCurrentTransform() * Matrix::MakeTranslation(position));

  text_contents.Render(renderer_, entity, *render_passes_.back());
}

void EntityCanvas::DrawLine(const Point& p0,
                            const Point& p1,
                            Color color,
                            BlendMode blend_mode,
                            Scalar width,
                            Cap cap) {
  LineGeometry geom(p0, p1, width, cap);
  SolidColorContents color_source;
  color_source.SetRawGeometry(&geom);
  color_source.SetColor(color);

  Draw(color_source, blend_mode);
}

void EntityCanvas::DrawRRect(const Rect& rect,
                             const Size& corner_radii,
                             Color color,
                             BlendMode blend_mode) {
  auto geom = RoundRectGeometry(rect, corner_radii);
  SolidColorContents color_source;
  color_source.SetRawGeometry(&geom);
  color_source.SetColor(color);

  Draw(color_source, blend_mode);
}

void EntityCanvas::Draw(const ColorSourceContents& color_source,
                        BlendMode blend_mode) {
  Entity entity;
  entity.SetTransform(
      Matrix::MakeTranslation(Vector3(-GetGlobalPassPosition())) *
      GetCurrentTransform());
  entity.SetClipDepth(GetClipDepth());
  entity.SetBlendMode(blend_mode);

  color_source.Render(renderer_, entity, *render_passes_.back());
}

}  // namespace impeller
