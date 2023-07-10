// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/impeller/renderer/backend/gles/device_buffer_gles.h"
#include "flutter/impeller/renderer/backend/gles/reactor_gles.h"
#include "flutter/impeller/renderer/render_pass.h"

namespace impeller {

/// @brief A class that caches the last set state when encoding gl commands to
/// avoid expensive and redundant bindings.
class PassBindingsCache {
 public:
  explicit PassBindingsCache(const ProcTableGLES& gl);

  PassBindingsCache(const PassBindingsCache&) = delete;

  PassBindingsCache(PassBindingsCache&&) = delete;

  bool Reset();
  bool ConfigurePipeline(
      std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline);
  void ConfigureDepth(std::optional<DepthAttachmentDescriptor> depth);

  bool BindVertexBuffer(BufferView vertex_buffer_view,
                        const std::shared_ptr<Allocator>& transients_allocator);
  bool BindIndexBuffer(BufferView index_buffer_view,
                       const std::shared_ptr<Allocator>& transients_allocator);
  void ConfigureScissor(std::optional<IRect> maybe_scissor, ISize target_size);
  void ConfigureViewport(Viewport viewport, ISize target_size, bool has_depth);
  void SetCullMode(CullMode cull_mode);
  void SetWindingOrder(WindingOrder winding_order);

 private:
  bool BindBuffer(BufferView buffer_view,
                  DeviceBufferGLES::BindingType binding_type,
                  const std::shared_ptr<Allocator>& transients_allocator);

  const ProcTableGLES& gl_;

  std::shared_ptr<Pipeline<PipelineDescriptor>> last_pipeline_ = nullptr;
  CullMode last_cull_mode_ = CullMode::kNone;
  WindingOrder last_winding_order_ = WindingOrder::kClockwise;
  Viewport last_viewport_ = Viewport{Rect{}, DepthRange{}};
  ISize last_target_size_ = ISize{};

  bool viewport_had_depth_ = false;
  bool had_depth_ = false;
  bool had_scissor_ = false;
  std::unordered_map<int, std::shared_ptr<const Buffer>> bound_buffers_ = {};
};

class RenderPassGLES final
    : public RenderPass,
      public std::enable_shared_from_this<RenderPassGLES> {
 public:
  // |RenderPass|
  ~RenderPassGLES() override;

 private:
  friend class CommandBufferGLES;

  ReactorGLES::Ref reactor_;
  std::string label_;
  bool is_valid_ = false;

  RenderPassGLES(std::weak_ptr<const Context> context,
                 const RenderTarget& target,
                 ReactorGLES::Ref reactor);

  // |RenderPass|
  bool IsValid() const override;

  // |RenderPass|
  void OnSetLabel(std::string label) override;

  // |RenderPass|
  bool OnEncodeCommands(const Context& context) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(RenderPassGLES);
};

}  // namespace impeller
