// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/shader_function_mtl.h"
#include "fml/trace_event.h"
#include "impeller/base/thread.h"
#include "impeller/base/thread_safety.h"

namespace impeller {

class PendingMTLShader {
 public:
  explicit PendingMTLShader(std::future<id<MTLFunction>> pending_shader)
      : pending_shader_(std::move(pending_shader)) {}

  id<MTLFunction> GetMTLFunction() {
    TRACE_EVENT0("flutter", "GetMTLFunction");
    Lock lock(mutex_);
    if (did_load_) {
      return loaded_function_;
    }
    did_load_ = true;
    loaded_function_ = pending_shader_.get();
    return loaded_function_;
  }

 private:
  Mutex mutex_;
  std::future<id<MTLFunction>> pending_shader_ IPLR_GUARDED_BY(mutex_);
  id<MTLFunction> loaded_function_ = nil;
  bool did_load_ = false;
};

ShaderFunctionMTL::ShaderFunctionMTL(
    UniqueID parent_library_id,
    std::future<id<MTLFunction>> pending_shader,
    std::string name,
    ShaderStage stage)
    : ShaderFunction(parent_library_id, std::move(name), stage),
      pending_function_(
          std::make_unique<PendingMTLShader>(std::move(pending_shader))) {}

ShaderFunctionMTL::~ShaderFunctionMTL() = default;

id<MTLFunction> ShaderFunctionMTL::GetMTLFunction() const {
  return pending_function_->GetMTLFunction();
}

}  // namespace impeller
