// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "flutter/fml/hash_combine.h"
#include "flutter/fml/macros.h"
#include "impeller/base/comparable.h"
#include "impeller/renderer/formats.h"
#include "impeller/renderer/shader_types.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

class ShaderFunction;
class VertexDescriptor;
class Pipeline;

class PipelineDescriptor final : public Comparable<PipelineDescriptor> {
 public:
  PipelineDescriptor();

  ~PipelineDescriptor();

  PipelineDescriptor& SetLabel(std::string label);

  const std::string& GetLabel() const;

  PipelineDescriptor& SetSampleCount(SampleCount samples);

  SampleCount GetSampleCount() const { return sample_count_; }

  PipelineDescriptor& AddStageEntrypoint(
      std::shared_ptr<const ShaderFunction> function);

  const std::map<ShaderStage, std::shared_ptr<const ShaderFunction>>&
  GetStageEntrypoints() const;

  std::shared_ptr<const ShaderFunction> GetEntrypointForStage(
      ShaderStage stage) const;

  PipelineDescriptor& SetVertexDescriptor(
      std::shared_ptr<VertexDescriptor> vertex_descriptor);

  const std::shared_ptr<VertexDescriptor>& GetVertexDescriptor() const;

  PipelineDescriptor& SetColorAttachmentDescriptor(
      size_t index,
      ColorAttachmentDescriptor desc);

  PipelineDescriptor& SetColorAttachmentDescriptors(
      std::map<size_t /* index */, ColorAttachmentDescriptor> descriptors);

  const ColorAttachmentDescriptor* GetColorAttachmentDescriptor(
      size_t index) const;

  const std::map<size_t /* index */, ColorAttachmentDescriptor>&
  GetColorAttachmentDescriptors() const;

  const ColorAttachmentDescriptor* GetLegacyCompatibleColorAttachment() const;

  PipelineDescriptor& SetDepthStencilAttachmentDescriptor(
      DepthAttachmentDescriptor desc);

  std::optional<DepthAttachmentDescriptor> GetDepthStencilAttachmentDescriptor()
      const;

  PipelineDescriptor& SetStencilAttachmentDescriptors(
      StencilAttachmentDescriptor front_and_back);

  PipelineDescriptor& SetStencilAttachmentDescriptors(
      StencilAttachmentDescriptor front,
      StencilAttachmentDescriptor back);

  std::optional<StencilAttachmentDescriptor>
  GetFrontStencilAttachmentDescriptor() const;

  std::optional<StencilAttachmentDescriptor>
  GetBackStencilAttachmentDescriptor() const;

  bool HasStencilAttachmentDescriptors() const;

  PipelineDescriptor& SetDepthPixelFormat(PixelFormat format);

  PixelFormat GetDepthPixelFormat() const;

  PipelineDescriptor& SetStencilPixelFormat(PixelFormat format);

  PixelFormat GetStencilPixelFormat() const;

  // Comparable<PipelineDescriptor>
  std::size_t GetHash() const override;

  // Comparable<PipelineDescriptor>
  bool IsEqual(const PipelineDescriptor& other) const override;

  void ResetAttachments();

  void SetCullMode(CullMode mode);

  CullMode GetCullMode() const;

  void SetWindingOrder(WindingOrder order);

  WindingOrder GetWindingOrder() const;

 private:
  std::string label_;
  SampleCount sample_count_ = SampleCount::kCount1;
  WindingOrder winding_order_ = WindingOrder::kClockwise;
  CullMode cull_mode_ = CullMode::kNone;
  std::map<ShaderStage, std::shared_ptr<const ShaderFunction>> entrypoints_;
  std::map<size_t /* index */, ColorAttachmentDescriptor>
      color_attachment_descriptors_;
  std::shared_ptr<VertexDescriptor> vertex_descriptor_;
  PixelFormat depth_pixel_format_ = PixelFormat::kUnknown;
  PixelFormat stencil_pixel_format_ = PixelFormat::kUnknown;
  std::optional<DepthAttachmentDescriptor> depth_attachment_descriptor_;
  std::optional<StencilAttachmentDescriptor>
      front_stencil_attachment_descriptor_;
  std::optional<StencilAttachmentDescriptor>
      back_stencil_attachment_descriptor_;
};

using PipelineMap =
    std::unordered_map<PipelineDescriptor,
                       std::shared_future<std::shared_ptr<Pipeline>>,
                       ComparableHash<PipelineDescriptor>,
                       ComparableEqual<PipelineDescriptor>>;

}  // namespace impeller
