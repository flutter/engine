// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_VK_H_

#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/surface.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      A collection of surfaces shared with the system compositor to
///             present images in the windowing system on the target platform.
///
class SwapchainVK {
 public:
  SwapchainVK();

  virtual ~SwapchainVK();

  SwapchainVK(const SwapchainVK&) = delete;

  SwapchainVK& operator=(const SwapchainVK&) = delete;

  //----------------------------------------------------------------------------
  /// @brief      If this instance of the swapchain is currently valid or must
  ///             be discarded.
  ///
  /// @return     True if valid, False otherwise.
  ///
  virtual bool IsValid() const = 0;

  //----------------------------------------------------------------------------
  /// @brief      The format of the images in this swapchain.
  ///
  /// @return     The format.
  ///
  virtual vk::Format GetSurfaceFormat() const = 0;

  //----------------------------------------------------------------------------
  /// @brief      Acquire the next swapchain image as a fully configured surface
  ///             to draw into.
  ///
  ///             This might be a blocking call as the swapchain may need to
  ///             wait for the image to become available after the compositor is
  ///             done using it.
  ///
  /// @return     The next swapchain image.
  ///
  virtual std::shared_ptr<Surface> AcquireNextDrawable() = 0;

  //----------------------------------------------------------------------------
  /// @brief      Mark the current swapchain configuration as dirty, forcing it
  ///             to be recreated on the next frame.
  ///
  /// @param[in]  size  The new surface size in pixels
  ///
  virtual void UpdateSurfaceSize(const ISize& size) = 0;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_SWAPCHAIN_VK_H_
