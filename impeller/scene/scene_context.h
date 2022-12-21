// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "impeller/renderer/context.h"
#include "impeller/renderer/pipeline.h"
#include "impeller/renderer/pipeline_descriptor.h"
#include "impeller/scene/pipeline_key.h"

namespace impeller {
namespace scene {

struct SceneContextOptions {
  SampleCount sample_count = SampleCount::kCount1;
  PrimitiveType primitive_type = PrimitiveType::kTriangle;

  struct Hash {
    constexpr std::size_t operator()(const SceneContextOptions& o) const {
      return fml::HashCombine(o.sample_count, o.primitive_type);
    }
  };

  struct Equal {
    constexpr bool operator()(const SceneContextOptions& lhs,
                              const SceneContextOptions& rhs) const {
      return lhs.sample_count == rhs.sample_count &&
             lhs.primitive_type == rhs.primitive_type;
    }
  };

  void ApplyToPipelineDescriptor(PipelineDescriptor& desc) const;
};

class SceneContext {
 public:
  explicit SceneContext(std::shared_ptr<Context> context);

  ~SceneContext();

  bool IsValid() const;

  std::shared_ptr<Pipeline<PipelineDescriptor>> GetPipeline(
      PipelineKey key,
      SceneContextOptions opts) const;

  std::shared_ptr<Context> GetContext() const;

  std::shared_ptr<Texture> GetPlaceholderTexture() const;

 private:
  template <class T>
  using Variants = std::unordered_map<SceneContextOptions,
                                      std::unique_ptr<T>,
                                      SceneContextOptions::Hash,
                                      SceneContextOptions::Equal>;

  class PipelineCollection {
   public:
    virtual ~PipelineCollection() = default;

    virtual std::shared_ptr<Pipeline<PipelineDescriptor>> GetPipeline(
        SceneContextOptions opts) = 0;
  };

  template <class PipelineT>
  class PipelineCollectionT final : public PipelineCollection {
   public:
    explicit PipelineCollectionT(Context& context) {
      auto desc = PipelineT::Builder::MakeDefaultPipelineDescriptor(context);
      // Apply default ContentContextOptions to the descriptor.
      SceneContextOptions{}.ApplyToPipelineDescriptor(*desc);
      pipelines_[{}] = std::make_unique<PipelineT>(context, desc);
    };

    // |PipelineCollection|
    std::shared_ptr<Pipeline<PipelineDescriptor>> GetPipeline(
        SceneContextOptions opts) {
      if (auto found = pipelines_.find(opts); found != pipelines_.end()) {
        return found->second->WaitAndGet();
      }

      auto prototype = pipelines_.find({});

      // The prototype must always be initialized in the constructor.
      FML_CHECK(prototype != pipelines_.end());

      auto variant_future = prototype->second->WaitAndGet()->CreateVariant(
          [&opts,
           variants_count = pipelines_.size()](PipelineDescriptor& desc) {
            opts.ApplyToPipelineDescriptor(desc);
            desc.SetLabel(
                SPrintF("%s V#%zu", desc.GetLabel().c_str(), variants_count));
          });
      auto variant = std::make_unique<PipelineT>(std::move(variant_future));
      auto variant_pipeline = variant->WaitAndGet();
      pipelines_[opts] = std::move(variant);
      return variant_pipeline;
    }

   private:
    Variants<PipelineT> pipelines_;
  };

  template <typename VertexShader, typename FragmentShader>
  std::unique_ptr<PipelineCollection> MakePipelineCollection(Context& context) {
    return std::make_unique<
        PipelineCollectionT<RenderPipelineT<VertexShader, FragmentShader>>>(
        context);
  }

  std::unordered_map<PipelineKey,
                     std::unique_ptr<PipelineCollection>,
                     PipelineKey::Hash,
                     PipelineKey::Equal>
      pipelines_;

  std::shared_ptr<Context> context_;

  bool is_valid_ = false;
  // A 1x1 opaque white texture that can be used as a placeholder binding.
  // Available for the lifetime of the scene context
  std::shared_ptr<Texture> placeholder_texture_;

  FML_DISALLOW_COPY_AND_ASSIGN(SceneContext);
};

}  // namespace scene
}  // namespace impeller
