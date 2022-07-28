// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/renderer/backend/vulkan/vk.h"

namespace impeller {

class DebugContextVK {
 public:
  explicit DebugContextVK(const vk::Device& logical_device);

  ~DebugContextVK();

  template <typename T>
  bool SetDebugName(T handle, std::string_view label) const {
    uint64_t handle_ptr =
        reinterpret_cast<uint64_t>(static_cast<typename T::NativeType>(handle));

    std::string label_str = std::string(label);

    auto ret = logical_device_.setDebugUtilsObjectNameEXT(
        vk::DebugUtilsObjectNameInfoEXT()
            .setObjectType(T::objectType)
            .setObjectHandle(handle_ptr)
            .setPObjectName(label_str.c_str()));

    if (ret != vk::Result::eSuccess) {
      VALIDATION_LOG << "unable to set debug name";
      return false;
    }

    return true;
  }

 private:
  const vk::Device& logical_device_;
};

}  // namespace impeller
