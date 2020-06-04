// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <thread>

#include "flutter/fml/message_loop.h"
#include "flutter/fml/raster_thread_merger.h"
#include "flutter/fml/synchronization/count_down_latch.h"
#include "flutter/fml/task_runner.h"
#include "flutter/shell/platform/android/external_view_embedder/external_view_embedder.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(AndroidExternalViewEmbedder, GetCurrentCanvases) {
  auto embedder = new AndroidExternalViewEmbedder();

  embedder->BeginFrame(SkISize::Make(10, 20), nullptr, 1.0);

  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());
  embedder->PrerollCompositeEmbeddedView(
      1, std::make_unique<EmbeddedViewParams>());

  auto canvases = embedder->GetCurrentCanvases();
  ASSERT_EQ(2UL, canvases.size());
  ASSERT_EQ(SkISize::Make(10, 20), canvases[0]->getBaseLayerSize());
  ASSERT_EQ(SkISize::Make(10, 20), canvases[1]->getBaseLayerSize());
}

TEST(AndroidExternalViewEmbedder, CompositeEmbeddedView) {
  auto embedder = new AndroidExternalViewEmbedder();

  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());
  ASSERT_TRUE(embedder->CompositeEmbeddedView(0) != nullptr);

  embedder->PrerollCompositeEmbeddedView(
      1, std::make_unique<EmbeddedViewParams>());
  ASSERT_TRUE(embedder->CompositeEmbeddedView(1) != nullptr);
}

TEST(AndroidExternalViewEmbedder, CancelFrame) {
  auto embedder = new AndroidExternalViewEmbedder();

  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());
  embedder->CancelFrame();

  auto canvases = embedder->GetCurrentCanvases();
  ASSERT_EQ(0UL, canvases.size());
}

TEST(AndroidExternalViewEmbedder, RasterizerRunsOnPlatformThread) {
  auto embedder = new AndroidExternalViewEmbedder();

  embedder->BeginFrame(SkISize::Make(10, 20), nullptr, 1.0);

  // Push a platform view.
  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());

  fml::MessageLoop* loop1 = nullptr;
  fml::AutoResetWaitableEvent latch1;
  fml::AutoResetWaitableEvent term1;
  std::thread thread1([&loop1, &latch1, &term1]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop1 = &fml::MessageLoop::GetCurrent();
    latch1.Signal();
    term1.Wait();
  });

  fml::MessageLoop* loop2 = nullptr;
  fml::AutoResetWaitableEvent latch2;
  fml::AutoResetWaitableEvent term2;
  std::thread thread2([&loop2, &latch2, &term2]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop2 = &fml::MessageLoop::GetCurrent();
    latch2.Signal();
    term2.Wait();
  });

  latch1.Wait();
  latch2.Wait();

  fml::TaskQueueId qid1 = loop1->GetTaskRunner()->GetTaskQueueId();
  fml::TaskQueueId qid2 = loop2->GetTaskRunner()->GetTaskQueueId();

  const auto raster_thread_merger =
      fml::MakeRefCounted<fml::RasterThreadMerger>(qid1, qid2);
  ASSERT_FALSE(raster_thread_merger->IsMerged());

  const auto postpreroll_result =
      embedder->PostPrerollAction(raster_thread_merger);
  ASSERT_EQ(PostPrerollResult::kResubmitFrame, postpreroll_result);
  ASSERT_FALSE(embedder->SubmitFrame(nullptr, nullptr));

  embedder->EndFrame(raster_thread_merger);
  ASSERT_TRUE(raster_thread_merger->IsMerged());

  int pending_frames = 0;
  while (raster_thread_merger->IsMerged()) {
    raster_thread_merger->DecrementLease();
    pending_frames++;
  }
  ASSERT_EQ(10, pending_frames);  // kDefaultMergedLeaseDuration

  term1.Signal();
  term2.Signal();
  thread1.join();
  thread2.join();
}

TEST(AndroidExternalViewEmbedder, RasterizerRunsOnRasterizerThread) {
  auto embedder = new AndroidExternalViewEmbedder();

  fml::MessageLoop* loop1 = nullptr;
  fml::AutoResetWaitableEvent latch1;
  fml::AutoResetWaitableEvent term1;
  std::thread thread1([&loop1, &latch1, &term1]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop1 = &fml::MessageLoop::GetCurrent();
    latch1.Signal();
    term1.Wait();
  });

  fml::MessageLoop* loop2 = nullptr;
  fml::AutoResetWaitableEvent latch2;
  fml::AutoResetWaitableEvent term2;
  std::thread thread2([&loop2, &latch2, &term2]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop2 = &fml::MessageLoop::GetCurrent();
    latch2.Signal();
    term2.Wait();
  });

  latch1.Wait();
  latch2.Wait();

  fml::TaskQueueId qid1 = loop1->GetTaskRunner()->GetTaskQueueId();
  fml::TaskQueueId qid2 = loop2->GetTaskRunner()->GetTaskQueueId();

  const auto raster_thread_merger =
      fml::MakeRefCounted<fml::RasterThreadMerger>(qid1, qid2);
  ASSERT_FALSE(raster_thread_merger->IsMerged());

  PostPrerollResult result = embedder->PostPrerollAction(raster_thread_merger);
  ASSERT_EQ(PostPrerollResult::kSuccess, result);

  embedder->EndFrame(raster_thread_merger);
  ASSERT_FALSE(raster_thread_merger->IsMerged());

  term1.Signal();
  term2.Signal();
  thread1.join();
  thread2.join();
}

}  // namespace testing
}  // namespace flutter
