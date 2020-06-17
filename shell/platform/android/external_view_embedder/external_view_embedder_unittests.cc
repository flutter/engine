// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/raster_thread_merger.h"
#include "flutter/fml/thread.h"
#include "flutter/shell/platform/android/external_view_embedder/external_view_embedder.h"
#include "flutter/shell/platform/android/jni/mock_jni.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

using ::testing::Mock;

TEST(AndroidExternalViewEmbedder, GetCurrentCanvases) {
  auto mock_jni = std::make_shared<MockJNI>();

  EXPECT_CALL(*mock_jni, FlutterViewBeginFrame());

  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, mock_jni, nullptr);
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

TEST(AndroidExternalViewEmbedder, GetCurrentCanvases__CompositeOrder) {
  auto mock_jni = std::make_shared<MockJNI>();
  EXPECT_CALL(*mock_jni, FlutterViewBeginFrame());

  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, mock_jni, nullptr);
  embedder->BeginFrame(SkISize::Make(10, 20), nullptr, 1.0);

  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());
  embedder->PrerollCompositeEmbeddedView(
      1, std::make_unique<EmbeddedViewParams>());

  auto canvases = embedder->GetCurrentCanvases();
  ASSERT_EQ(2UL, canvases.size());
  ASSERT_EQ(embedder->CompositeEmbeddedView(0), canvases[0]);
  ASSERT_EQ(embedder->CompositeEmbeddedView(1), canvases[1]);
}

TEST(AndroidExternalViewEmbedder, CompositeEmbeddedView) {
  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, nullptr, nullptr);

  ASSERT_EQ(nullptr, embedder->CompositeEmbeddedView(0));
  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());
  ASSERT_NE(nullptr, embedder->CompositeEmbeddedView(0));

  ASSERT_EQ(nullptr, embedder->CompositeEmbeddedView(1));
  embedder->PrerollCompositeEmbeddedView(
      1, std::make_unique<EmbeddedViewParams>());
  ASSERT_NE(nullptr, embedder->CompositeEmbeddedView(1));
}

TEST(AndroidExternalViewEmbedder, CancelFrame) {
  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, nullptr, nullptr);

  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());
  embedder->CancelFrame();

  auto canvases = embedder->GetCurrentCanvases();
  ASSERT_EQ(0UL, canvases.size());
}

TEST(AndroidExternalViewEmbedder, RasterizerRunsOnPlatformThread) {
  auto mock_jni = std::make_shared<MockJNI>();
  EXPECT_CALL(*mock_jni, FlutterViewBeginFrame());
  EXPECT_CALL(*mock_jni, FlutterViewEndFrame());

  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, mock_jni, nullptr);
  auto platform_thread = new fml::Thread("platform");
  auto rasterizer_thread = new fml::Thread("rasterizer");
  auto platform_queue_id = platform_thread->GetTaskRunner()->GetTaskQueueId();
  auto rasterizer_queue_id =
      rasterizer_thread->GetTaskRunner()->GetTaskQueueId();

  auto raster_thread_merger = fml::MakeRefCounted<fml::RasterThreadMerger>(
      platform_queue_id, rasterizer_queue_id);
  ASSERT_FALSE(raster_thread_merger->IsMerged());

  embedder->BeginFrame(SkISize::Make(10, 20), nullptr, 1.0);
  // Push a platform view.
  embedder->PrerollCompositeEmbeddedView(
      0, std::make_unique<EmbeddedViewParams>());

  auto postpreroll_result = embedder->PostPrerollAction(raster_thread_merger);
  ASSERT_EQ(PostPrerollResult::kResubmitFrame, postpreroll_result);
  ASSERT_TRUE(embedder->SubmitFrame(nullptr, nullptr));

  embedder->EndFrame(raster_thread_merger);
  ASSERT_TRUE(raster_thread_merger->IsMerged());

  int pending_frames = 0;
  while (raster_thread_merger->IsMerged()) {
    raster_thread_merger->DecrementLease();
    pending_frames++;
  }
  ASSERT_EQ(10, pending_frames);  // kDefaultMergedLeaseDuration
}

TEST(AndroidExternalViewEmbedder, RasterizerRunsOnRasterizerThread) {
  auto mock_jni = std::make_shared<MockJNI>();
  EXPECT_CALL(*mock_jni, FlutterViewEndFrame());

  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, mock_jni, nullptr);
  auto platform_thread = new fml::Thread("platform");
  auto rasterizer_thread = new fml::Thread("rasterizer");
  auto platform_queue_id = platform_thread->GetTaskRunner()->GetTaskQueueId();
  auto rasterizer_queue_id =
      rasterizer_thread->GetTaskRunner()->GetTaskQueueId();

  auto raster_thread_merger = fml::MakeRefCounted<fml::RasterThreadMerger>(
      platform_queue_id, rasterizer_queue_id);
  ASSERT_FALSE(raster_thread_merger->IsMerged());

  PostPrerollResult result = embedder->PostPrerollAction(raster_thread_merger);
  ASSERT_EQ(PostPrerollResult::kSuccess, result);

  embedder->EndFrame(raster_thread_merger);
  ASSERT_FALSE(raster_thread_merger->IsMerged());
}

TEST(AndroidExternalViewEmbedder, PlatformViewRect) {
  auto mock_jni = std::make_shared<MockJNI>();
  EXPECT_CALL(*mock_jni, FlutterViewBeginFrame());

  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, mock_jni, nullptr);
  embedder->BeginFrame(SkISize::Make(100, 100), nullptr, 1.5);

  auto view_params = std::make_unique<EmbeddedViewParams>();
  view_params->offsetPixels = SkPoint::Make(10, 20);
  view_params->sizePoints = SkSize::Make(30, 40);

  auto view_id = 0;
  embedder->PrerollCompositeEmbeddedView(view_id, std::move(view_params));
  ASSERT_EQ(SkRect::MakeXYWH(10, 20, 45, 60), embedder->GetViewRect(view_id));
}

TEST(AndroidExternalViewEmbedder, PlatformViewRect__ChangedParams) {
  auto mock_jni = std::make_shared<MockJNI>();
  EXPECT_CALL(*mock_jni, FlutterViewBeginFrame());

  auto embedder =
      std::make_unique<AndroidExternalViewEmbedder>(nullptr, mock_jni, nullptr);
  embedder->BeginFrame(SkISize::Make(100, 100), nullptr, 1.5);

  auto view_id = 0;
  auto view_params_1 = std::make_unique<EmbeddedViewParams>();
  view_params_1->offsetPixels = SkPoint::Make(10, 20);
  view_params_1->sizePoints = SkSize::Make(30, 40);
  embedder->PrerollCompositeEmbeddedView(view_id, std::move(view_params_1));

  auto view_params_2 = std::make_unique<EmbeddedViewParams>();
  view_params_2->offsetPixels = SkPoint::Make(50, 60);
  view_params_2->sizePoints = SkSize::Make(70, 80);
  embedder->PrerollCompositeEmbeddedView(view_id, std::move(view_params_2));

  ASSERT_EQ(SkRect::MakeXYWH(50, 60, 105, 120), embedder->GetViewRect(view_id));
}

}  // namespace testing
}  // namespace flutter
