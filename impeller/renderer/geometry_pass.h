// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <variant>

#include "impeller/core/device_buffer.h"
#include "impeller/core/texture.h"
#include "impeller/renderer/compute_command.h"
#include "impeller/renderer/compute_pass.h"

namespace impeller {

class RenderPass;
class ContentContext;

struct GeometryPassResult {
  BufferView indirect_command_arguments;
  BufferView output_geometry;
};

class DevicePrivateBuffer final
    : public std::enable_shared_from_this<DevicePrivateBuffer>,
      public Buffer {
 public:
  static std::shared_ptr<DevicePrivateBuffer> Create();

  // |Buffer|
  virtual ~DevicePrivateBuffer();

  void SetLabel(std::string label);

  [[nodiscard]] BufferView AsBufferView();

  [[nodiscard]] BufferView Reserve(size_t length);

 private:
  mutable std::shared_ptr<DeviceBuffer> device_buffer_;
  size_t size_ = 0u;
  mutable size_t device_buffer_generation_ = 0u;
  size_t generation_ = 1u;
  std::string label_;

  // |Buffer|
  std::shared_ptr<const DeviceBuffer> GetDeviceBuffer(
      Allocator& allocator) const override;

  DevicePrivateBuffer();

  FML_DISALLOW_COPY_AND_ASSIGN(DevicePrivateBuffer);
};

class GeometryPass {
 public:
  GeometryPass();

  /// Add a polyline to the current geometry pass.
  ///
  /// This will return a BufferView that contains an IndirectCommandArguments
  /// for indirect command encoding. The compute pass will be processed before
  /// the current render pass is encoded.
  GeometryPassResult AddPolyline(Path::Polyline polyline,
                                 const ContentContext& renderer);

  bool Encode(ComputePass& pass);

 private:
  struct AccumulatedConvexCommand {
    uint32_t count;
    uint32_t size;
    std::shared_ptr<HostBuffer> input_buffer;
    std::shared_ptr<HostBuffer> indirect_buffer;
    std::shared_ptr<HostBuffer> index_buffer;
    std::shared_ptr<DevicePrivateBuffer> output_buffer;
  };

  AccumulatedConvexCommand& GetOrCreateConvex(size_t count);

  std::vector<AccumulatedConvexCommand> convex_commands_;

  std::shared_ptr<Pipeline<ComputePipelineDescriptor>> convex_pipeline_;

  FML_DISALLOW_COPY_AND_ASSIGN(GeometryPass);
};

}  // namespace impeller
