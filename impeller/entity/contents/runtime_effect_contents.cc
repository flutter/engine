// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/runtime_effect_contents.h"

#include <future>

#include "flutter/fml/make_copyable.h"
#include "fml/logging.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/renderer/shader_types.h"

namespace impeller {

bool RuntimeEffectContents::Render(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) const {
  std::promise<bool> promise;
  auto future = promise.get_future();

  auto context = renderer.GetContext();
  auto library = context->GetShaderLibrary();
  library->RegisterFunction(
      runtime_stage_->GetEntrypoint(),
      ToShaderStage(runtime_stage_->GetShaderStage()),
      runtime_stage_->GetCodeMapping(),
      fml::MakeCopyable([promise = std::move(promise)](bool result) mutable {
        promise.set_value(result);
      }));

  if (!future.get()) {
    FML_LOG(ERROR) << "Failed to build runtime stage (entry point:"
                   << runtime_stage_->GetEntrypoint() << ")";
    return false;
  }

  return true;
}

}  // namespace impeller