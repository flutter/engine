// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/rasterizer.h"

#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"
#include "gmock/gmock.h"

using testing::_;
using testing::Return;
using testing::ReturnRef;

namespace flutter {
namespace {
class MockDelegate : public Rasterizer::Delegate {
 public:
  MOCK_METHOD(void,
              OnFrameRasterized,
              (const FrameTiming& frame_timing),
              (override));
  MOCK_METHOD(fml::Milliseconds, GetFrameBudget, (), (override));
  MOCK_METHOD(fml::TimePoint, GetLatestFrameTargetTime, (), (const, override));
  MOCK_METHOD(const TaskRunners&, GetTaskRunners, (), (const, override));
  MOCK_METHOD(std::shared_ptr<fml::SyncSwitch>,
              GetIsGpuDisabledSyncSwitch,
              (),
              (const, override));
};

class MockSurface : public Surface {
 public:
  MOCK_METHOD(bool, IsValid, (), (override));
  MOCK_METHOD(std::unique_ptr<SurfaceFrame>,
              AcquireFrame,
              (const SkISize& size),
              (override));
  MOCK_METHOD(SkMatrix, GetRootTransformation, (), (const, override));
  MOCK_METHOD(GrDirectContext*, GetContext, (), (override));
  MOCK_METHOD(ExternalViewEmbedder*, GetExternalViewEmbedder, (), (override));
  MOCK_METHOD(std::unique_ptr<GLContextResult>,
              MakeRenderContextCurrent,
              (),
              (override));
  MOCK_METHOD(bool, ClearRenderContext, (), (override));
};

class MockExternalViewEmbedder : public ExternalViewEmbedder {
 public:
  MOCK_METHOD(SkCanvas*, GetRootCanvas, (), (override));
  MOCK_METHOD(void, CancelFrame, (), (override));
  MOCK_METHOD(void,
              BeginFrame,
              (SkISize frame_size,
               GrDirectContext* context,
               double device_pixel_ratio,
               fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger),
              (override));
  MOCK_METHOD(void,
              PrerollCompositeEmbeddedView,
              (int view_id, std::unique_ptr<EmbeddedViewParams> params),
              (override));
  MOCK_METHOD(PostPrerollResult,
              PostPrerollAction,
              (fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger),
              (override));
  MOCK_METHOD(std::vector<SkCanvas*>, GetCurrentCanvases, (), (override));
  MOCK_METHOD(SkCanvas*, CompositeEmbeddedView, (int view_id), (override));
  MOCK_METHOD(void,
              SubmitFrame,
              (GrDirectContext * context, std::unique_ptr<SurfaceFrame> frame),
              (override));
  MOCK_METHOD(void,
              EndFrame,
              (bool should_resubmit_frame,
               fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger),
              (override));
  MOCK_METHOD(bool, SupportsDynamicThreadMerging, (), (override));
};
}  // namespace

TEST(RasterizerTest, create) {
  MockDelegate delegate;
  auto rasterizer = std::make_unique<Rasterizer>(delegate);
  EXPECT_TRUE(rasterizer != nullptr);
}

TEST(RasterizerTest, drawEmptyPipeline) {
  std::string test_name =
      ::testing::UnitTest::GetInstance()->current_test_info()->name();
  ThreadHost thread_host("io.flutter.test." + test_name + ".",
                         ThreadHost::Type::Platform | ThreadHost::Type::GPU |
                             ThreadHost::Type::IO | ThreadHost::Type::UI);
  TaskRunners task_runners("test", thread_host.platform_thread->GetTaskRunner(),
                           thread_host.raster_thread->GetTaskRunner(),
                           thread_host.ui_thread->GetTaskRunner(),
                           thread_host.io_thread->GetTaskRunner());
  MockDelegate delegate;
  EXPECT_CALL(delegate, GetTaskRunners()).WillOnce(ReturnRef(task_runners));
  auto rasterizer = std::make_unique<Rasterizer>(delegate);
  auto surface = std::make_unique<MockSurface>();
  rasterizer->Setup(std::move(surface));
  fml::AutoResetWaitableEvent latch;
  thread_host.raster_thread->GetTaskRunner()->PostTask([&] {
    auto pipeline = fml::AdoptRef(new Pipeline<LayerTree>(/*depth=*/10));
    rasterizer->Draw(pipeline, nullptr);
    latch.Signal();
  });
  latch.Wait();
}

TEST(RasterizerTest, drawWithExternalViewEmbedder) {
  std::string test_name =
      ::testing::UnitTest::GetInstance()->current_test_info()->name();
  ThreadHost thread_host("io.flutter.test." + test_name + ".",
                         ThreadHost::Type::Platform | ThreadHost::Type::GPU |
                             ThreadHost::Type::IO | ThreadHost::Type::UI);
  TaskRunners task_runners("test", thread_host.platform_thread->GetTaskRunner(),
                           thread_host.raster_thread->GetTaskRunner(),
                           thread_host.ui_thread->GetTaskRunner(),
                           thread_host.io_thread->GetTaskRunner());
  MockDelegate delegate;
  EXPECT_CALL(delegate, GetTaskRunners())
      .WillRepeatedly(ReturnRef(task_runners));
  EXPECT_CALL(delegate, OnFrameRasterized(_));
  auto rasterizer = std::make_unique<Rasterizer>(delegate);
  auto surface = std::make_unique<MockSurface>();
  MockExternalViewEmbedder external_view_embedder;
  EXPECT_CALL(*surface, GetExternalViewEmbedder())
      .WillRepeatedly(Return(&external_view_embedder));
  EXPECT_CALL(external_view_embedder,
              BeginFrame(SkISize(), nullptr, 2.0,
                         fml::RefPtr<fml::RasterThreadMerger>(nullptr)));
  EXPECT_CALL(external_view_embedder,
              EndFrame(false, fml::RefPtr<fml::RasterThreadMerger>(nullptr)));
  rasterizer->Setup(std::move(surface));
  fml::AutoResetWaitableEvent latch;
  thread_host.raster_thread->GetTaskRunner()->PostTask([&] {
    auto pipeline = fml::AdoptRef(new Pipeline<LayerTree>(/*depth=*/10));
    auto layer_tree = std::make_unique<LayerTree>(/*frame_size=*/SkISize(),
                                                  /*device_pixel_ratio=*/2.0f);
    bool result = pipeline->Produce().Complete(std::move(layer_tree));
    EXPECT_TRUE(result);
    std::function<bool(LayerTree&)> no_discard = [](LayerTree&) {
      return false;
    };
    rasterizer->Draw(pipeline, no_discard);
    latch.Signal();
  });
  latch.Wait();
}
}  // namespace flutter
