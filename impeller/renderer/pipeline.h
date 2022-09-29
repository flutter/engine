// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <future>

#include "compute_pipeline_descriptor.h"
#include "flutter/fml/macros.h"
#include "impeller/renderer/compute_pipeline_builder.h"
#include "impeller/renderer/compute_pipeline_descriptor.h"
#include "impeller/renderer/context.h"
#include "impeller/renderer/pipeline_builder.h"
#include "impeller/renderer/pipeline_descriptor.h"

namespace impeller {

class PipelineLibrary;
template <typename PipelineDescriptor_>
class Pipeline;

// TODO(csg): Using a simple future is sub-optimal since callers that want to
// eagerly create and cache pipeline variants will have to await on the future
// to get its pipeline descriptor (unless they have explicitly cached it). This
// would be a concurrency pessimization.
//
// Use a struct that stores the future and the descriptor separately.
template <class T>
using PipelineFuture = std::shared_future<std::shared_ptr<Pipeline<T>>>;

//------------------------------------------------------------------------------
/// @brief      Describes the fixed function and programmable aspects of
///             rendering and compute operations performed by commands submitted
///             to the GPU via a command buffer.
///
///             A pipeline handle must be allocated upfront and kept alive for
///             as long as possible. Do not create a pipeline object within a
///             frame workload.
///
///             This pipeline object is almost never used directly as it is
///             untyped. Use reflected shader information generated by the
///             Impeller offline shader compiler to generate a typed pipeline
///             object.
///
template <typename T>
class Pipeline {
 public:
  virtual ~Pipeline();

  virtual bool IsValid() const = 0;

  //----------------------------------------------------------------------------
  /// @brief      Get the descriptor that was responsible for creating this
  ///             pipeline. It may be copied and modified to create a pipeline
  ///             variant.
  ///
  /// @return     The descriptor.
  ///
  const T& GetDescriptor() const;

  PipelineFuture<T> CreateVariant(
      std::function<void(T& desc)> descriptor_callback) const;

 protected:
  Pipeline(std::weak_ptr<PipelineLibrary> library, T desc);

 private:
  const std::weak_ptr<PipelineLibrary> library_;
  const T desc_;

  FML_DISALLOW_COPY_AND_ASSIGN(Pipeline);
};

extern template class Pipeline<PipelineDescriptor>;
extern template class Pipeline<ComputePipelineDescriptor>;

PipelineFuture<PipelineDescriptor> CreatePipelineFuture(
    const Context& context,
    std::optional<PipelineDescriptor> desc);

PipelineFuture<ComputePipelineDescriptor> CreatePipelineFuture(
    const Context& context,
    std::optional<ComputePipelineDescriptor> desc);

template <class VertexShader_, class FragmentShader_>
class RenderPipelineT {
 public:
  using VertexShader = VertexShader_;
  using FragmentShader = FragmentShader_;
  using Builder = PipelineBuilder<VertexShader, FragmentShader>;

  explicit RenderPipelineT(const Context& context)
      : RenderPipelineT(CreatePipelineFuture(
            context,
            Builder::MakeDefaultPipelineDescriptor(context))) {}

  explicit RenderPipelineT(const Context& context,
                           std::optional<PipelineDescriptor> desc)
      : RenderPipelineT(CreatePipelineFuture(context, desc)) {}

  explicit RenderPipelineT(PipelineFuture<PipelineDescriptor> future)
      : pipeline_future_(std::move(future)) {}

  std::shared_ptr<Pipeline<PipelineDescriptor>> WaitAndGet() {
    if (did_wait_) {
      return pipeline_;
    }
    did_wait_ = true;
    if (pipeline_future_.valid()) {
      pipeline_ = pipeline_future_.get();
    }
    return pipeline_;
  }

 private:
  PipelineFuture<PipelineDescriptor> pipeline_future_;
  std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline_;
  bool did_wait_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(RenderPipelineT);
};

template <class ComputeShader_>
class ComputePipelineT {
 public:
  using ComputeShader = ComputeShader_;
  using Builder = ComputePipelineBuilder<ComputeShader>;

  explicit ComputePipelineT(const Context& context)
      : ComputePipelineT(CreatePipelineFuture(
            context,
            Builder::MakeDefaultPipelineDescriptor(context))) {}

  explicit ComputePipelineT(
      const Context& context,
      std::optional<ComputePipelineDescriptor> compute_desc)
      : ComputePipelineT(CreatePipelineFuture(context, compute_desc)) {}

  explicit ComputePipelineT(PipelineFuture<ComputePipelineDescriptor> future)
      : pipeline_future_(std::move(future)) {}

  std::shared_ptr<Pipeline<ComputePipelineDescriptor>> WaitAndGet() {
    if (did_wait_) {
      return pipeline_;
    }
    did_wait_ = true;
    if (pipeline_future_.valid()) {
      pipeline_ = pipeline_future_.get();
    }
    return pipeline_;
  }

 private:
  PipelineFuture<ComputePipelineDescriptor> pipeline_future_;
  std::shared_ptr<Pipeline<ComputePipelineDescriptor>> pipeline_;
  bool did_wait_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(ComputePipelineT);
};

}  // namespace impeller
