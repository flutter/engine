// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/renderer/backend/vulkan/context_vk.h"

namespace impeller {

class BackgroundContextVK : public ContextVK {
public:
  virtual BackendType GetBackendType() const {

  }

  virtual std::string DescribeGpuModel() const = 0;

  virtual bool IsValid() const = 0;

  virtual const std::shared_ptr<const Capabilities>& GetCapabilities()
      const = 0;

  virtual bool UpdateOffscreenLayerPixelFormat(PixelFormat format);

  virtual std::shared_ptr<Allocator> GetResourceAllocator() const = 0;

  virtual std::shared_ptr<ShaderLibrary> GetShaderLibrary() const = 0;

  virtual std::shared_ptr<SamplerLibrary> GetSamplerLibrary() const = 0;

  virtual std::shared_ptr<PipelineLibrary> GetPipelineLibrary() const = 0;

  virtual std::shared_ptr<CommandBuffer> CreateCommandBuffer() const = 0;

  virtual void Shutdown() = 0;

  virtual std::shared_ptr<DeviceHolder> GetDeviceHolder() const = 0;

  virtual vk::Instance GetInstance() const = 0;

  virtual const vk::Device& GetDevice() const = 0;

  virtual const std::shared_ptr<fml::ConcurrentTaskRunner>
  GetConcurrentWorkerTaskRunner() const = 0;

  virtual std::shared_ptr<SurfaceContextVK> CreateSurfaceContext() = 0;

  virtual const std::shared_ptr<QueueVK>& GetGraphicsQueue() const = 0;

  virtual vk::PhysicalDevice GetPhysicalDevice() const = 0;

  virtual std::shared_ptr<FenceWaiterVK> GetFenceWaiter() const = 0;

  virtual std::shared_ptr<ResourceManagerVK> GetResourceManager() const = 0;

private:
  std::weak_ptr<const ContextVK> root_context_;
};
}
