// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_PIPELINE_CACHE_DATA_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_PIPELINE_CACHE_DATA_VK_H_

#include "flutter/fml/mapping.h"
#include "flutter/fml/unique_fd.h"
#include "impeller/renderer/backend/vulkan/vk.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      Persist the pipeline cache to a file in the given cache
///             directory. This function performs integrity checks the Vulkan
///             driver may have missed.
///
/// @warning    The pipeline cache must be externally synchronized for most
///             complete results. If additional pipelines are being created
///             while this function is executing, this function may fail to
///             persist data.
///
/// @param[in]  cache_directory  The cache directory
/// @param[in]  props            The physical device properties
/// @param[in]  cache            The cache
///
/// @return     If the cache data could be persisted to disk.
///
bool PipelineCacheDataPersist(const fml::UniqueFD& cache_directory,
                              const VkPhysicalDeviceProperties& props,
                              const vk::UniquePipelineCache& cache);

//------------------------------------------------------------------------------
/// @brief      Retrieve the previously persisted pipeline cache data. This
///             function provides integrity checks the Vulkan driver may have
///             missed.
///
/// @param[in]  cache_directory  The cache directory
/// @param[in]  props            The properties
///
/// @return     The cache data if it was found and checked to have passed
///             additional integrity checks.
///
std::unique_ptr<fml::Mapping> PipelineCacheDataRetrieve(
    const fml::UniqueFD& cache_directory,
    const VkPhysicalDeviceProperties& props);

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_PIPELINE_CACHE_DATA_VK_H_
