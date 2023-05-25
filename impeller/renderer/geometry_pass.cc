// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/geometry_pass.h"

#include <iostream>
#include <tuple>

#include "impeller/core/host_buffer.h"
#include "impeller/entity/contents/content_context.h"  // nogncheck
#include "impeller/entity/convex.comp.h"               // nogncheck
#include "impeller/renderer/render_pass.h"

namespace impeller {

std::shared_ptr<DevicePrivateBuffer> DevicePrivateBuffer::Create() {
  return std::shared_ptr<DevicePrivateBuffer>(new DevicePrivateBuffer());
}

DevicePrivateBuffer::DevicePrivateBuffer() = default;

DevicePrivateBuffer::~DevicePrivateBuffer() = default;

void DevicePrivateBuffer::SetLabel(std::string label) {
  label_ = std::move(label);
}

BufferView DevicePrivateBuffer::AsBufferView() {
  return BufferView{shared_from_this(), nullptr, Range{0, size_}};
}

BufferView DevicePrivateBuffer::Reserve(size_t length) {
  auto old_length = size_;
  size_ += length;
  generation_++;
  return BufferView{shared_from_this(), nullptr, Range{old_length, length}};
}

std::shared_ptr<const DeviceBuffer> DevicePrivateBuffer::GetDeviceBuffer(
    Allocator& allocator) const {
  if (generation_ == device_buffer_generation_) {
    return device_buffer_;
  }
  DeviceBufferDescriptor desc;
  desc.storage_mode = StorageMode::kDevicePrivate;
  desc.size = size_;

  auto new_buffer = allocator.CreateBuffer(desc);
  if (!new_buffer) {
    return nullptr;
  }
  new_buffer->SetLabel(label_);
  device_buffer_generation_ = generation_;
  device_buffer_ = std::move(new_buffer);
  return device_buffer_;
}

GeometryPass::GeometryPass() {}

constexpr size_t kMaxConvexSegments = 0xFFFFFFFF;  // max uint32_t.

GeometryPass::AccumulatedConvexCommand& GeometryPass::GetOrCreateConvex(
    size_t count) {
  FML_DCHECK(count <= kMaxConvexSegments);
  if (convex_commands_.size() == 0 ||
      (convex_commands_.size() + count > kMaxConvexSegments)) {
    convex_commands_.emplace_back(AccumulatedConvexCommand{
        .count = 0u,
        .size = 0u,
        .input_buffer = HostBuffer::Create(),
        .indirect_buffer = HostBuffer::Create(),
        .index_buffer = HostBuffer::Create(),
        .output_buffer = DevicePrivateBuffer::Create(),
    });
  }
  return convex_commands_.back();
}

GeometryPassResult GeometryPass::AddPolyline(Path::Polyline polyline,
                                             const ContentContext& renderer) {
  using CS = ConvexComputeShader;
  auto result_size = polyline.points.size() * 3;
  convex_pipeline_ = renderer.GetConvexComputePipeline();
  auto& convex_command = GetOrCreateConvex(polyline.points.size());

  // Emplace with no padding as we're going to treat this as a single buffer.
  std::ignore = convex_command.input_buffer->Emplace(
      polyline.points.data(), polyline.points.size() * sizeof(Point), 0);

  auto geometry_buffer =
      convex_command.output_buffer->Reserve(result_size * sizeof(Point));

  // TODO: find a smarter way to do this?
  std::vector<CS::IndexDataItem> index_data(polyline.points.size());
  for (auto i = 0u; i < polyline.points.size(); i++) {
    index_data[i] = CS::IndexDataItem{
        .first_offset = convex_command.size,
        .indirect_offset = convex_command.count,
    };
  }
  std::ignore = convex_command.index_buffer->Emplace(
      index_data.data(), polyline.points.size() * sizeof(CS::IndexDataItem), 0);

  convex_command.count += 1;
  convex_command.size += polyline.points.size();
  return {
      .indirect_command_arguments = {},
      .output_geometry = geometry_buffer,
  };
}

bool GeometryPass::Encode(ComputePass& pass) {
  using CS = ConvexComputeShader;
  for (const auto& accumulated : convex_commands_) {
    ComputeCommand cmd;
    cmd.label = "Convex Geometry";
    cmd.pipeline = convex_pipeline_;
    cmd.grid_size = ISize(accumulated.size, 1);

    CS::FrameData frame_data;
    frame_data.input_count = accumulated.size;

    CS::BindFrameData(cmd,
                      pass.GetTransientsBuffer().EmplaceUniform(frame_data));
    CS::BindGeometry(cmd, accumulated.output_buffer->AsBufferView());
    CS::BindPolyline(cmd, accumulated.input_buffer->AsBufferView());
    CS::BindIndexData(cmd, accumulated.index_buffer->AsBufferView());

    if (!pass.AddCommand(std::move(cmd))) {
      return false;
    }
  }
  return true;
}

}  // namespace impeller
