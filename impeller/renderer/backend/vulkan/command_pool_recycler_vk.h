// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <vector>
#include "fml/macros.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      Creates and manages the lifecycle of |CommandPoolVK|s.
///
/// Creating a command pool attempts to reuse an existing pool, and if none
/// exists, creates a new one. When a pool is no longer needed, it is recycled
/// in a background thread and made available for reuse across any thread.
///
/// A single |CommandPoolRecyclerVK| should be created for each |ContextVK|.
///
/// This class is thread-safe.
///
/// @note       This is an experimental API, and has some duplicate
///             functionality with |CommandPoolVK::GetThreadLocal|.
///
/// @see        https://github.com/flutter/flutter/issues/133198
class CommandPoolRecyclerVK final {
 public:
  /// @brief      Creates a |CommandPoolRecyclerVK|.
  ///
  /// @param[in]  context  The |ContextVK| to use.
  static const std::unique_ptr<CommandPoolRecyclerVK> Create(
      const std::shared_ptr<ContextVK>& context);

  /// @brief      Gets the |CommandPoolVK| for the current thread.
  std::shared_ptr<class CommandPoolVK> Get();

  /// @brief      Recycles the current |CommandPoolVK| for the current thread.
  ///
  /// @param[in]  context  The |ContextVK| to use.
  ///
  /// If the current thread has a |CommandPoolVK|, it will be recycled in a
  /// background thread and made available for reuse in a future call to
  /// |GetThreadLocal|.
  void Recycle();

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(CommandPoolRecyclerVK);

  explicit CommandPoolRecyclerVK(const std::shared_ptr<ContextVK>& context);
  ~CommandPoolRecyclerVK();

  /// @brief      Creates a new |CommandPoolVK| or reuses an existing one.
  ///
  /// @return     The |CommandPoolVK|, or |nullptr| if one could not be created.
  std::shared_ptr<CommandPoolVK> CreateOrReusePool();

  // The context for the current recycler.
  const std::shared_ptr<ContextVK> context_;

  // Pools that are not currently in use and can be provided to |Create| calls.
  std::vector<std::shared_ptr<CommandPoolVK>> recycled_pools_;
};

}  // namespace impeller
