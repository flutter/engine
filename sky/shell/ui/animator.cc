// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/sky/shell/ui/animator.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/trace_event/trace_event.h"
#include "flutter/common/threads.h"

namespace sky {
namespace shell {

Animator::Animator(Rasterizer* rasterizer, Engine* engine)
    : rasterizer_(rasterizer),
      engine_(engine),
      layer_tree_pipeline_(3),
      pending_frame_sem_(1),
      paused_(false),
      weak_factory_(this) {}

Animator::~Animator() = default;

void Animator::Stop() {
  paused_ = true;
}

void Animator::Start() {
  if (!paused_) {
    return;
  }

  paused_ = false;
  RequestFrame();
}

void Animator::BeginFrame(int64_t time_stamp) {
  if (paused_) {
    return;
  }

  pending_frame_sem_.Signal();

  LayerTreePipeline::Producer producer = [&]() {
    renderable_tree_.reset();
    engine_->BeginFrame(ftl::TimePoint::Now());
    auto result = std::move(renderable_tree_);
    renderable_tree_.reset();
    return result;
  };

  if (!layer_tree_pipeline_.Produce(producer)) {
    TRACE_EVENT_INSTANT0("flutter", "ConsumerSlowDefer",
                         TRACE_EVENT_SCOPE_PROCESS);
    RequestFrame();
    return;
  }

  blink::Threads::Gpu()->PostTask(
      [&]() { rasterizer_->Draw(&layer_tree_pipeline_); });
}

void Animator::Render(std::unique_ptr<flow::LayerTree> layer_tree) {
  renderable_tree_ = std::move(layer_tree);
}

void Animator::RequestFrame() {
  if (paused_) {
    return;
  }

  if (!pending_frame_sem_.TryWait()) {
    // Multiple calls to Animator::RequestFrame will still result in a single
    // request to the VSyncProvider.
    return;
  }

  // The AwaitVSync is going to call us back at the next VSync. However, we want
  // to be reasonably certain that the UI thread is not in the middle of a
  // particularly expensive callout. We post the AwaitVSync to run right after
  // an idle. This does NOT provide a guarantee that the UI thread has not
  // started an expensive operation right after posting this message however.
  // To support that, we need edge triggered wakes on VSync.

  blink::Threads::UI()->PostTask([&]() {
    if (paused_) {
      return;
    }

    TRACE_EVENT_INSTANT0("flutter", "RequestFrame", TRACE_EVENT_SCOPE_PROCESS);

    if (vsync_provider_) {
      vsync_provider_->AwaitVSync(
          base::Bind(&Animator::BeginFrame, weak_factory_.GetWeakPtr()));
      return;
    }

    CHECK(false) << "Animator setup incomplete. A vsync provider or frame "
                    "schedule must be present.";
  });
}

void Animator::set_vsync_provider(vsync::VSyncProviderPtr vsync_provider) {
  vsync_provider_ = vsync_provider.Pass();
  RequestFrame();
}

}  // namespace shell
}  // namespace sky
