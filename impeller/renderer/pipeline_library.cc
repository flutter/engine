// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/pipeline_library.h"

namespace impeller {

PipelineLibrary::PipelineLibrary() = default;

PipelineLibrary::~PipelineLibrary() = default;

PipelineFuture<PipelineDescriptor> PipelineLibrary::GetPipeline(
    std::optional<PipelineDescriptor> descriptor) {
  if (descriptor.has_value()) {
    return GetPipeline(descriptor.value());
  }
  auto promise = std::make_shared<
      std::promise<std::shared_ptr<Pipeline<PipelineDescriptor>>>>();
  promise->set_value(nullptr);
  return {descriptor, promise->get_future()};
}

PipelineFuture<ComputePipelineDescriptor> PipelineLibrary::GetPipeline(
    std::optional<ComputePipelineDescriptor> descriptor) {
  if (descriptor.has_value()) {
    return GetPipeline(descriptor.value());
  }
  auto promise = std::make_shared<
      std::promise<std::shared_ptr<Pipeline<ComputePipelineDescriptor>>>>();
  promise->set_value(nullptr);
  return {descriptor, promise->get_future()};
}

std::vector<PipelineFuture<PipelineDescriptor>> PipelineLibrary::GetPipelines(
    const std::vector<PipelineDescriptor>& descriptors) {
  std::vector<PipelineFuture<PipelineDescriptor>> result;
  result.reserve(descriptors.size());
  for (const PipelineDescriptor& descriptor : descriptors) {
    result.emplace_back(GetPipeline(descriptor));
  }
  return result;
}

}  // namespace impeller
