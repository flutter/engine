// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "solid_color_contents.h"

#include "impeller/entity/contents/clip_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/path.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

SolidColorContents::SolidColorContents() = default;

SolidColorContents::~SolidColorContents() = default;

void SolidColorContents::SetColor(Color color) {
  color_ = color;
}

const Color& SolidColorContents::GetColor() const {
  return color_;
}

void SolidColorContents::SetGeometry(std::shared_ptr<Geometry> geometry) {
  geometry_ = std::move(geometry);
}

std::optional<Rect> SolidColorContents::GetCoverage(
    const Entity& entity) const {
  if (color_.IsTransparent()) {
    return std::nullopt;
  }
  if (geometry_ == nullptr) {
    return std::nullopt;
  }
  return geometry_->GetCoverage(entity.GetTransformation());
};

std::optional<Snapshot> SolidColorContents::RenderToSnapshot(
    const ContentContext& renderer,
    const Entity& entity,
    const std::optional<SamplerDescriptor>& sampler_descriptor,
    bool msaa_enabled) const {
  auto coverage = GetCoverage(entity);
  if (!coverage.has_value()) {
    return std::nullopt;
  }

  impeller::TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = impeller::StorageMode::kHostVisible;
  texture_descriptor.format = PixelFormat::kR8G8B8A8UNormInt;
  texture_descriptor.size = {1, 1};
  auto texture = renderer.GetContext()->GetResourceAllocator()->CreateTexture(
      texture_descriptor);
  if (!texture) {
    FML_DLOG(ERROR) << "Could not create Impeller texture.";
    return std::nullopt;
  }

  std::vector<uint8_t> data(4);
  auto raw_color = color_.Premultiply().ToR8G8B8A8();
  data[0] = raw_color[0];
  data[1] = raw_color[1];
  data[2] = raw_color[2];
  data[3] = raw_color[3];
  auto mapping = std::make_shared<fml::DataMapping>(data);
  if (!texture->SetContents(mapping)) {
    FML_DLOG(ERROR) << "Could not copy contents into Impeller texture.";
    return std::nullopt;
  }
  texture->SetLabel(impeller::SPrintF("SolidColorSnapshot"));

  auto size = coverage->size;
  auto origin = coverage->origin;
  // clang-format off
  auto tsx = Matrix(
    size.width, 0.0,         0.0, 0.0,
    0.0,        size.height, 0.0, 0.0,
    0.0,        0.0,         1.0, 0.0,
    origin.x,   origin.y,    0.0, 1.0
  );
  // clang-format on
  return Snapshot{.texture = texture, .transform = tsx};
}

bool SolidColorContents::ShouldRender(
    const Entity& entity,
    const std::optional<Rect>& stencil_coverage) const {
  if (!stencil_coverage.has_value()) {
    return false;
  }
  return Contents::ShouldRender(entity, stencil_coverage);
}

bool SolidColorContents::Render(const ContentContext& renderer,
                                const Entity& entity,
                                RenderPass& pass) const {
  using VS = SolidFillPipeline::VertexShader;
  using FS = SolidFillPipeline::FragmentShader;

  Command cmd;
  cmd.label = "Solid Fill";
  cmd.stencil_reference = entity.GetStencilDepth();

  auto geometry_result = geometry_->GetPositionBuffer(renderer, entity, pass);

  auto options = OptionsFromPassAndEntity(pass, entity);
  if (geometry_result.prevent_overdraw) {
    options.stencil_compare = CompareFunction::kEqual;
    options.stencil_operation = StencilOperation::kIncrementClamp;
  }

  options.primitive_type = geometry_result.type;
  cmd.pipeline = renderer.GetSolidFillPipeline(options);
  cmd.BindVertices(geometry_result.vertex_buffer);

  VS::FrameInfo frame_info;
  frame_info.mvp = geometry_result.transform;
  VS::BindFrameInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frame_info));

  FS::FragInfo frag_info;
  frag_info.color = color_.Premultiply();
  FS::BindFragInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frag_info));

  if (!pass.AddCommand(std::move(cmd))) {
    return false;
  }

  if (geometry_result.prevent_overdraw) {
    auto restore = ClipRestoreContents();
    restore.SetRestoreCoverage(GetCoverage(entity));
    return restore.Render(renderer, entity, pass);
  }
  return true;
}

std::unique_ptr<SolidColorContents> SolidColorContents::Make(const Path& path,
                                                             Color color) {
  auto contents = std::make_unique<SolidColorContents>();
  contents->SetGeometry(Geometry::MakeFillPath(path));
  contents->SetColor(color);
  return contents;
}

}  // namespace impeller
