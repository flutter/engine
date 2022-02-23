// Copyright 2022 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "test_component.h"

#include <lib/fidl/cpp/interface_request.h>
#include <lib/sys/cpp/component_context.h>

#include "flutter/fml/logging.h"
#include "suite.h"

TestComponent::TestComponent(TestComponentArgs args, DoneCallback done_callback)
    : dispatcher_(args.dispatcher),
      binding_(this, std::move(args.request), args.dispatcher),
      done_callback_(std::move(done_callback)) {
  suite_ = std::make_unique<Suite>(std::move(args.parent_env_svc),
                                   std::move(args.parent_env),
                                   std::move(args.test_component_svc),
                                   std::move(args.legacy_url), dispatcher_);
  suite_context_ = sys::ComponentContext::Create();
  suite_context_->outgoing()->AddPublicService(suite_->GetHandler());
  suite_context_->outgoing()->Serve(std::move(args.outgoing_dir), dispatcher_);
}

TestComponent::~TestComponent() = default;

void TestComponent::Stop() {
  Kill();
}

void TestComponent::Kill() {
  binding_.Close(ZX_OK);
  // this object would be killed after this call
  done_callback_(this);
}
