// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "flutter/fml/concurrent_message_loop.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"
#include "flutter/fml/unique_fd.h"
#include "impeller/base/backend_cast.h"
#include "impeller/core/formats.h"
#include "impeller/renderer/backend/vulkan/device_holder.h"
#include "impeller/renderer/backend/vulkan/pipeline_library_vk.h"
#include "impeller/renderer/backend/vulkan/queue_vk.h"
#include "impeller/renderer/backend/vulkan/sampler_library_vk.h"
#include "impeller/renderer/backend/vulkan/shader_library_vk.h"
#include "impeller/renderer/backend/vulkan/shared_object_vk.h"
#include "impeller/renderer/capabilities.h"
#include "impeller/renderer/context.h"

namespace impeller {

bool HasValidationLayers();

class CommandEncoderFactoryVK;
class CommandEncoderVK;
class DebugReportVK;
class FenceWaiterVK;
class ResourceManagerVK;
class SurfaceContextVK;

/// @brief Ring buffer for one writer thread and one reader thread.
template <typename T>
class RingBuffer {
 public:
  RingBuffer(int32_t size) : read_(0), write_(0), buffer_(size) {}

  std::optional<T> Read() {
    int32_t current_read = read_.load();
    if (current_read == write_.load()) {
      return {};
    } else {
      int32_t next_read = (current_read + 1) % buffer_.size();
      read_.store(next_read);
      return buffer_[current_read];
    }
  }

  bool Write(const T& value) {
    int32_t current_write = write_.load();
    int32_t next_write = (current_write + 1) % buffer_.size();
    if (next_write == read_.load()) {  // Buffer full
      return false;
    }
    buffer_[current_write] = value;
    write_.store(next_write);
    return true;
  }

 private:
  std::atomic<int32_t> read_;
  std::atomic<int32_t> write_;
  std::vector<T> buffer_;
};

template <typename T>
class RingBufferItem : public SharedObjectVK {
 public:
  RingBufferItem(std::weak_ptr<const ContextVK> context,
                 RingBuffer<T>* ring_buffer,
                 std::optional<T> value)
      : context_(context), ring_buffer_(ring_buffer), value_(value) {}

  virtual ~RingBufferItem() {
    auto context = context_.lock();
    if (value_ && context) {
      bool did_write = ring_buffer_->Write(value_.value());
      FML_CHECK(did_write);
    }
  }

  std::optional<T>& GetValue() { return value_; }

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(RingBufferItem<T>);

  std::weak_ptr<const ContextVK> context_;
  RingBuffer<T>* ring_buffer_;
  std::optional<T> value_;
};

struct RenderPassFrameBufferPair {
  SharedHandleVK<vk::RenderPass> render_pass;
  SharedHandleVK<vk::Framebuffer> framebuffer;
};

class ContextVK final : public Context,
                        public BackendCast<ContextVK, Context>,
                        public std::enable_shared_from_this<ContextVK> {
 public:
  struct Settings {
    PFN_vkGetInstanceProcAddr proc_address_callback = nullptr;
    std::vector<std::shared_ptr<fml::Mapping>> shader_libraries_data;
    fml::UniqueFD cache_directory;
    bool enable_validation = false;

    Settings() = default;

    Settings(Settings&&) = default;
  };

  static std::shared_ptr<ContextVK> Create(Settings settings);

  uint64_t GetHash() const { return hash_; }

  // |Context|
  ~ContextVK() override;

  // |Context|
  BackendType GetBackendType() const override;

  // |Context|
  std::string DescribeGpuModel() const override;

  // |Context|
  bool IsValid() const override;

  // |Context|
  std::shared_ptr<Allocator> GetResourceAllocator() const override;

  // |Context|
  std::shared_ptr<ShaderLibrary> GetShaderLibrary() const override;

  // |Context|
  std::shared_ptr<SamplerLibrary> GetSamplerLibrary() const override;

  // |Context|
  std::shared_ptr<PipelineLibrary> GetPipelineLibrary() const override;

  // |Context|
  std::shared_ptr<CommandBuffer> CreateCommandBuffer() const override;

  // |Context|
  const std::shared_ptr<const Capabilities>& GetCapabilities() const override;

  // |Context|
  void Shutdown() override;

  void SetOffscreenFormat(PixelFormat pixel_format);

  template <typename T>
  bool SetDebugName(T handle, std::string_view label) const {
    return SetDebugName(GetDevice(), handle, label);
  }

  template <typename T>
  static bool SetDebugName(const vk::Device& device,
                           T handle,
                           std::string_view label) {
    if (!HasValidationLayers()) {
      // No-op if validation layers are not enabled.
      return true;
    }

    auto c_handle = static_cast<typename T::CType>(handle);

    vk::DebugUtilsObjectNameInfoEXT info;
    info.objectType = T::objectType;
    info.pObjectName = label.data();
    info.objectHandle = reinterpret_cast<decltype(info.objectHandle)>(c_handle);

    if (device.setDebugUtilsObjectNameEXT(info) != vk::Result::eSuccess) {
      VALIDATION_LOG << "Unable to set debug name: " << label;
      return false;
    }

    return true;
  }

  std::shared_ptr<DeviceHolder> GetDeviceHolder() const {
    return device_holder_;
  }

  vk::Instance GetInstance() const;

  const vk::Device& GetDevice() const;

  const std::shared_ptr<fml::ConcurrentTaskRunner>
  GetConcurrentWorkerTaskRunner() const;

  std::shared_ptr<SurfaceContextVK> CreateSurfaceContext();

  const std::shared_ptr<QueueVK>& GetGraphicsQueue() const;

  vk::PhysicalDevice GetPhysicalDevice() const;

  std::shared_ptr<FenceWaiterVK> GetFenceWaiter() const;

  std::shared_ptr<ResourceManagerVK> GetResourceManager() const;

  std::shared_ptr<RingBufferItem<RenderPassFrameBufferPair>>
  GetRenderPassFrameBufferPair() const {
    return std::make_shared<RingBufferItem<RenderPassFrameBufferPair>>(
        shared_from_this(), &render_pass_pool_, render_pass_pool_.Read());
  }

 private:
  struct DeviceHolderImpl : public DeviceHolder {
    // |DeviceHolder|
    const vk::Device& GetDevice() const override { return device.get(); }
    // |DeviceHolder|
    const vk::PhysicalDevice& GetPhysicalDevice() const override {
      return physical_device;
    }

    vk::UniqueInstance instance;
    vk::PhysicalDevice physical_device;
    vk::UniqueDevice device;
  };

  std::shared_ptr<DeviceHolderImpl> device_holder_;
  std::unique_ptr<DebugReportVK> debug_report_;
  std::shared_ptr<Allocator> allocator_;
  std::shared_ptr<ShaderLibraryVK> shader_library_;
  std::shared_ptr<SamplerLibraryVK> sampler_library_;
  std::shared_ptr<PipelineLibraryVK> pipeline_library_;
  QueuesVK queues_;
  std::shared_ptr<const Capabilities> device_capabilities_;
  std::shared_ptr<FenceWaiterVK> fence_waiter_;
  std::shared_ptr<ResourceManagerVK> resource_manager_;
  std::string device_name_;
  std::shared_ptr<fml::ConcurrentMessageLoop> raster_message_loop_;
  const uint64_t hash_;
  mutable RingBuffer<RenderPassFrameBufferPair> render_pass_pool_ =
      RingBuffer<RenderPassFrameBufferPair>(10);

  bool is_valid_ = false;

  ContextVK();

  void Setup(Settings settings);

  std::unique_ptr<CommandEncoderFactoryVK> CreateGraphicsCommandEncoderFactory()
      const;

  FML_DISALLOW_COPY_AND_ASSIGN(ContextVK);
};

}  // namespace impeller
