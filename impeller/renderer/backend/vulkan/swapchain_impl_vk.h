// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_IMPL_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_IMPL_VK_H_

#include <cstdint>
#include <memory>
#include <variant>

#include "impeller/geometry/size.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/surface.h"
#include "vulkan/vulkan_enums.hpp"

namespace impeller {

class Context;
class SwapchainImageVK;
struct FrameSynchronizer;

class SwapchainImplVK {
 public:
  static std::shared_ptr<SwapchainImplVK> Create(
      const std::shared_ptr<Context>& context,
      vk::UniqueSurfaceKHR surface,
      const ISize& size,
      bool enable_msaa = true,
      vk::SwapchainKHR old_swapchain = VK_NULL_HANDLE);

  SwapchainImplVK(const std::shared_ptr<Context>& context, const ISize& size);
  virtual ~SwapchainImplVK() = default;

  virtual bool IsValid() const = 0;
  struct AcquireResult {
    std::unique_ptr<Surface> surface;
    bool out_of_date = false;

    explicit AcquireResult(bool p_out_of_date = false)
        : out_of_date(p_out_of_date) {}

    explicit AcquireResult(std::unique_ptr<Surface> p_surface)
        : surface(std::move(p_surface)) {}
  };

  virtual AcquireResult AcquireNextDrawable() = 0;
  virtual vk::Format GetSurfaceFormat() const = 0;
  std::shared_ptr<Context> GetContext() const { return context_.lock(); }

  virtual std::shared_ptr<SwapchainImplVK> RecreateSwapchain() = 0;

  const ISize& GetSize() const { return size_; }

 protected:
  std::weak_ptr<Context> context_;

  void WaitIdle() const;

 private:
  ISize size_;
};

//------------------------------------------------------------------------------
/// @brief      An instance of a swapchain that does NOT adapt to going out of
///             date with the underlying surface. Errors will be indicated when
///             the next drawable is acquired from this implementation of the
///             swapchain. If the error is due the swapchain going out of date,
///             the caller must recreate another instance by optionally
///             stealing this implementations guts.
///
class DefaultSwapchainImplVK final
    : public std::enable_shared_from_this<DefaultSwapchainImplVK>,
      public SwapchainImplVK {
 public:
  ~DefaultSwapchainImplVK() override;

  // |SwapchainImplVK|
  bool IsValid() const override;

  // |SwapchainImplVK|
  AcquireResult AcquireNextDrawable() override;

  // |SwapchainImplVK|
  vk::Format GetSurfaceFormat() const override;

  // |SwapchainImplVK|
  std::shared_ptr<SwapchainImplVK> RecreateSwapchain() override;

 private:
  vk::UniqueSurfaceKHR surface_;
  vk::Format surface_format_ = vk::Format::eUndefined;
  vk::UniqueSwapchainKHR swapchain_;
  std::vector<std::shared_ptr<SwapchainImageVK>> images_;
  std::vector<std::unique_ptr<FrameSynchronizer>> synchronizers_;
  size_t current_frame_ = 0u;
  bool enable_msaa_ = true;
  bool is_valid_ = false;

  std::pair<vk::UniqueSurfaceKHR, vk::UniqueSwapchainKHR> DestroySwapchain();

  DefaultSwapchainImplVK(const std::shared_ptr<Context>& context,
                         vk::UniqueSurfaceKHR surface,
                         const ISize& size,
                         bool enable_msaa,
                         vk::SwapchainKHR old_swapchain);

  bool Present(const std::shared_ptr<SwapchainImageVK>& image, uint32_t index);

  DefaultSwapchainImplVK(const DefaultSwapchainImplVK&) = delete;

  DefaultSwapchainImplVK& operator=(const DefaultSwapchainImplVK&) = delete;

  friend class SwapchainImplVK;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_IMPL_VK_H_
