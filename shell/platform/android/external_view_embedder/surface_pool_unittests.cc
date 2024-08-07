// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include "flutter/shell/platform/android/external_view_embedder/surface_pool.h"

#include "flutter/fml/make_copyable.h"
#include "flutter/shell/platform/android/jni/jni_mock.h"
#include "flutter/shell/platform/android/surface/android_surface_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/core/SkSize.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {
namespace testing {

using ::testing::ByMove;
using ::testing::Return;

class TestAndroidSurfaceFactory : public AndroidSurfaceFactory {
 public:
  using TestSurfaceProducer =
      std::function<std::unique_ptr<AndroidSurface>(void)>;
  explicit TestAndroidSurfaceFactory(TestSurfaceProducer&& surface_producer) {
    surface_producer_ = surface_producer;
  }

  ~TestAndroidSurfaceFactory() override = default;

  std::unique_ptr<AndroidSurface> CreateSurface() override {
    return surface_producer_();
  }

 private:
  TestSurfaceProducer surface_producer_;
};

TEST(SurfacePool, GetLayerAllocateOneLayer) {
  auto pool = std::make_unique<SurfacePool>();

  auto gr_context = GrDirectContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);

  auto jni_mock = std::make_shared<JNIMock>();
  auto window = fml::MakeRefCounted<AndroidNativeWindow>(nullptr);
  EXPECT_CALL(*jni_mock, FlutterViewCreateOverlaySurface())
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              0, window))));

  auto surface_factory =
      std::make_shared<TestAndroidSurfaceFactory>([gr_context, window]() {
        auto android_surface_mock = std::make_unique<AndroidSurfaceMock>();
        EXPECT_CALL(*android_surface_mock, CreateGPUSurface(gr_context.get()));
        EXPECT_CALL(*android_surface_mock, SetNativeWindow(window));
        EXPECT_CALL(*android_surface_mock, IsValid()).WillOnce(Return(true));
        return android_surface_mock;
      });
  EXPECT_FALSE(pool->CheckLayerProperties(gr_context.get(), SkISize{100, 100}));
  pool->CreateLayer(gr_context.get(), *android_context, jni_mock,
                    surface_factory);
  auto layer = pool->GetNextLayer();

  EXPECT_TRUE(pool->size() > 0);
  EXPECT_NE(nullptr, layer);
}

TEST(SurfacePool, GetUnusedLayers) {
  auto pool = std::make_unique<SurfacePool>();

  auto gr_context = GrDirectContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);

  auto jni_mock = std::make_shared<JNIMock>();
  auto window = fml::MakeRefCounted<AndroidNativeWindow>(nullptr);
  EXPECT_CALL(*jni_mock, FlutterViewCreateOverlaySurface())
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              0, window))));

  auto surface_factory =
      std::make_shared<TestAndroidSurfaceFactory>([gr_context, window]() {
        auto android_surface_mock = std::make_unique<AndroidSurfaceMock>();
        EXPECT_CALL(*android_surface_mock, CreateGPUSurface(gr_context.get()));
        EXPECT_CALL(*android_surface_mock, SetNativeWindow(window));
        EXPECT_CALL(*android_surface_mock, IsValid()).WillOnce(Return(true));
        return android_surface_mock;
      });
  EXPECT_FALSE(pool->CheckLayerProperties(gr_context.get(), SkISize{100, 100}));
  pool->CreateLayer(gr_context.get(), *android_context, jni_mock,
                    surface_factory);
  EXPECT_EQ(0UL, pool->GetUnusedLayers().size());

  auto layer = pool->GetNextLayer();
  pool->RecycleLayers();

  EXPECT_EQ(pool->size(), 1u);
  EXPECT_EQ(1UL, pool->GetUnusedLayers().size());
  EXPECT_EQ(layer, pool->GetUnusedLayers()[0]);
}

TEST(SurfacePool, GetLayerRecycle) {
  auto pool = std::make_unique<SurfacePool>();

  auto gr_context_1 = GrDirectContext::MakeMock(nullptr);
  auto jni_mock = std::make_shared<JNIMock>();
  auto window = fml::MakeRefCounted<AndroidNativeWindow>(nullptr);
  EXPECT_CALL(*jni_mock, FlutterViewCreateOverlaySurface())
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              0, window))));

  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);

  auto gr_context_2 = GrDirectContext::MakeMock(nullptr);
  auto surface_factory = std::make_shared<TestAndroidSurfaceFactory>(
      [gr_context_1, gr_context_2, window]() {
        auto android_surface_mock = std::make_unique<AndroidSurfaceMock>();
        // Allocate two GPU surfaces for each gr context.
        EXPECT_CALL(*android_surface_mock,
                    CreateGPUSurface(gr_context_1.get()));
        EXPECT_CALL(*android_surface_mock,
                    CreateGPUSurface(gr_context_2.get()));
        // Set the native window once.
        EXPECT_CALL(*android_surface_mock, SetNativeWindow(window));
        EXPECT_CALL(*android_surface_mock, IsValid()).WillOnce(Return(true));
        return android_surface_mock;
      });
  EXPECT_FALSE(
      pool->CheckLayerProperties(gr_context_1.get(), SkISize{100, 100}));
  pool->CreateLayer(gr_context_1.get(), *android_context, jni_mock,
                    surface_factory);
  auto layer_1 = pool->GetNextLayer();
  pool->RecycleLayers();

  EXPECT_TRUE(
      pool->CheckLayerProperties(gr_context_2.get(), SkISize{100, 100}));
  pool->DestroyLayers(jni_mock);

  pool->CreateLayer(gr_context_2.get(), *android_context, jni_mock,
                    surface_factory);
  auto layer_2 = pool->GetNextLayer();

  EXPECT_TRUE(pool->size() > 0);
  EXPECT_NE(nullptr, layer_1);
  EXPECT_NE(layer_1, layer_2);
}

TEST(SurfacePool, GetLayerAllocateTwoLayers) {
  auto pool = std::make_unique<SurfacePool>();

  auto gr_context = GrDirectContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);

  auto jni_mock = std::make_shared<JNIMock>();
  auto window = fml::MakeRefCounted<AndroidNativeWindow>(nullptr);
  EXPECT_CALL(*jni_mock, FlutterViewCreateOverlaySurface())
      .Times(2)
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              0, window))))
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              1, window))));

  auto surface_factory =
      std::make_shared<TestAndroidSurfaceFactory>([gr_context, window]() {
        auto android_surface_mock = std::make_unique<AndroidSurfaceMock>();
        EXPECT_CALL(*android_surface_mock, CreateGPUSurface(gr_context.get()));
        EXPECT_CALL(*android_surface_mock, SetNativeWindow(window));
        EXPECT_CALL(*android_surface_mock, IsValid()).WillOnce(Return(true));
        return android_surface_mock;
      });

  EXPECT_FALSE(pool->CheckLayerProperties(gr_context.get(), SkISize{100, 100}));
  for (auto i = 0; i < 2; i++) {
    pool->CreateLayer(gr_context.get(), *android_context, jni_mock,
                      surface_factory);
  }
  auto layer_1 = pool->GetNextLayer();
  auto layer_2 = pool->GetNextLayer();

  EXPECT_EQ(pool->size(), 1u);
  EXPECT_NE(nullptr, layer_1);
  EXPECT_NE(nullptr, layer_2);
  EXPECT_NE(layer_1, layer_2);
  EXPECT_EQ(0, layer_1->id);
  EXPECT_EQ(1, layer_2->id);
}

TEST(SurfacePool, DestroyLayers) {
  auto pool = std::make_unique<SurfacePool>();
  auto jni_mock = std::make_shared<JNIMock>();

  EXPECT_CALL(*jni_mock, FlutterViewDestroyOverlaySurfaces()).Times(0);
  pool->DestroyLayers(jni_mock);

  auto gr_context = GrDirectContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);

  auto window = fml::MakeRefCounted<AndroidNativeWindow>(nullptr);
  EXPECT_CALL(*jni_mock, FlutterViewCreateOverlaySurface())
      .Times(1)
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              0, window))));

  auto surface_factory =
      std::make_shared<TestAndroidSurfaceFactory>([gr_context, window]() {
        auto android_surface_mock = std::make_unique<AndroidSurfaceMock>();
        EXPECT_CALL(*android_surface_mock, CreateGPUSurface(gr_context.get()));
        EXPECT_CALL(*android_surface_mock, SetNativeWindow(window));
        EXPECT_CALL(*android_surface_mock, IsValid()).WillOnce(Return(true));
        return android_surface_mock;
      });
  EXPECT_FALSE(pool->CheckLayerProperties(gr_context.get(), SkISize{100, 100}));
  pool->CreateLayer(gr_context.get(), *android_context, jni_mock,
                    surface_factory);

  EXPECT_CALL(*jni_mock, FlutterViewDestroyOverlaySurfaces());

  EXPECT_EQ(pool->size(), 1u);
  pool->DestroyLayers(jni_mock);

  EXPECT_EQ(pool->size(), 0u);
  ASSERT_TRUE(pool->GetUnusedLayers().empty());
}

TEST(SurfacePool, DestroyLayersFrameSizeChanged) {
  auto pool = std::make_unique<SurfacePool>();
  auto jni_mock = std::make_shared<JNIMock>();

  auto gr_context = GrDirectContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);

  auto window = fml::MakeRefCounted<AndroidNativeWindow>(nullptr);

  auto surface_factory =
      std::make_shared<TestAndroidSurfaceFactory>([gr_context, window]() {
        auto android_surface_mock = std::make_unique<AndroidSurfaceMock>();
        EXPECT_CALL(*android_surface_mock, CreateGPUSurface(gr_context.get()));
        EXPECT_CALL(*android_surface_mock, SetNativeWindow(window));
        EXPECT_CALL(*android_surface_mock, IsValid()).WillOnce(Return(true));
        return android_surface_mock;
      });
  EXPECT_FALSE(pool->CheckLayerProperties(gr_context.get(), SkISize{10, 10}));

  EXPECT_CALL(*jni_mock, FlutterViewDestroyOverlaySurfaces()).Times(0);
  EXPECT_CALL(*jni_mock, FlutterViewCreateOverlaySurface())
      .Times(1)
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              0, window))));

  EXPECT_EQ(pool->size(), 0u);
  pool->CreateLayer(gr_context.get(), *android_context, jni_mock,
                    surface_factory);

  EXPECT_EQ(pool->size(), 1u);
  EXPECT_TRUE(pool->CheckLayerProperties(gr_context.get(), SkISize{20, 20}));
  pool->DestroyLayers(jni_mock);

  EXPECT_CALL(*jni_mock, FlutterViewDestroyOverlaySurfaces()).Times(1);
  EXPECT_CALL(*jni_mock, FlutterViewCreateOverlaySurface())
      .Times(1)
      .WillOnce(Return(
          ByMove(std::make_unique<PlatformViewAndroidJNI::OverlayMetadata>(
              1, window))));

  pool->CreateLayer(gr_context.get(), *android_context, jni_mock,
                    surface_factory);
  pool->GetNextLayer();

  EXPECT_TRUE(pool->GetUnusedLayers().empty());
  EXPECT_EQ(pool->size(), 1u);
}

}  // namespace testing
}  // namespace flutter
