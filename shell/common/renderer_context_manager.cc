// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer_context_manager.h"

#include "flutter/fml/thread_local.h"

namespace flutter {

FML_THREAD_LOCAL
fml::ThreadLocalUniquePtr<std::vector<std::shared_ptr<RendererContext>>>
    context_stack;

RendererContext::RendererContext() = default;

RendererContext::~RendererContext() = default;

RendererContextManager::RendererContextManager(
    std::shared_ptr<RendererContext> context,
    std::shared_ptr<RendererContext> resource_context)
    : context_(context), resource_context_(resource_context){};

RendererContextManager::~RendererContextManager() = default;

std::unique_ptr<RendererContextSwitch> RendererContextManager::MakeCurrent(
    std::shared_ptr<RendererContext> context) {
  return std::make_unique<RendererContextSwitch>(*this, context);
};

std::unique_ptr<RendererContextSwitch>
RendererContextManager::FlutterMakeCurrent() {
  return MakeCurrent(context_);
};

std::unique_ptr<RendererContextSwitch>
RendererContextManager::FlutterResourceMakeCurrent() {
  return MakeCurrent(resource_context_);
};

bool RendererContextManager::PushContext(
    std::shared_ptr<RendererContext> context) {
  // Make sure the stack is initialized on the current thread.
  EnsureStackInitialized();
  bool result = context->SetCurrent();
  context_stack.get()->push_back(context);
  return result;
};

bool RendererContextManager::PopContext() {
  FML_CHECK(context_stack.get() != nullptr);
  FML_CHECK(!context_stack.get()->empty());
  bool result = context_stack.get()->back()->RemoveCurrent();
  if (context_stack.get()->size() == 1) {
    // only one context left, we remove the context
    context_stack.get()->pop_back();
    return result;
  }
  context_stack.get()->pop_back();
  return context_stack.get()->back()->SetCurrent();
};

void RendererContextManager::EnsureStackInitialized() {
  if (context_stack.get() == nullptr) {
    context_stack.reset(new std::vector<std::shared_ptr<RendererContext>>());
  }
}

RendererContextResult::RendererContextResult(bool static_result)
    : result_(static_result){};

bool RendererContextResult::GetResult() {
  return result_;
};

RendererContextResult::RendererContextResult() = default;
RendererContextResult::~RendererContextResult() = default;

RendererContextSwitch::RendererContextSwitch(
    RendererContextManager& manager,
    std::shared_ptr<RendererContext> context)
    : manager_(manager) {
  bool result = manager_.PushContext(context);
  result_ = result;
};

RendererContextSwitch::~RendererContextSwitch() {
  manager_.PopContext();
};

}  // namespace flutter
