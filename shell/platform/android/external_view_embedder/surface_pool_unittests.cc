// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/external_view_embedder/surface_pool.h"
#include "flutter/shell/platform/android/jni/mock_jni.h"
#include "flutter/shell/platform/android/surface/mock_android_surface.h"
#include "gtest/gtest.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace flutter {
namespace testing {

TEST(SurfacePool, GetLayer__AllocateOneLayer) {
  auto pool = new SurfacePool();

  auto gr_context = GrContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);
  auto mock_jni = std::make_shared<MockJNI>();
  auto surface_factory =
      [](std::shared_ptr<AndroidContext> android_context,
         std::shared_ptr<PlatformViewAndroidJNI> jni_facade) {
        return std::make_unique<MockAndroidSurface>(/*id=*/123);
      };
  auto layer = pool->GetLayer(gr_context.get(), android_context, mock_jni,
                              surface_factory);

  ASSERT_NE(nullptr, layer);
  ASSERT_EQ(gr_context.get(), layer->gr_context);
  ASSERT_NE(nullptr, layer->surface);
  ASSERT_EQ(gr_context.get(), layer->surface->GetContext());
  ASSERT_EQ(
      123,
      static_cast<MockAndroidSurface*>(layer->android_surface.get())->GetId());
}

TEST(SurfacePool, GetUnusedLayers) {
  auto pool = new SurfacePool();

  auto gr_context = GrContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);
  auto mock_jni = std::make_shared<MockJNI>();
  auto surface_factory =
      [](std::shared_ptr<AndroidContext> android_context,
         std::shared_ptr<PlatformViewAndroidJNI> jni_facade) {
        return std::make_unique<MockAndroidSurface>(/*id=*/123);
      };
  auto layer = pool->GetLayer(gr_context.get(), android_context, mock_jni,
                              surface_factory);
  ASSERT_EQ(0UL, pool->GetUnusedLayers().size());

  pool->RecycleLayers();
  ASSERT_EQ(1UL, pool->GetUnusedLayers().size());
  ASSERT_EQ(layer, pool->GetUnusedLayers()[0]);
}

TEST(SurfacePool, GetLayer__Recycle) {
  auto pool = new SurfacePool();

  auto gr_context_1 = GrContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);
  auto mock_jni = std::make_shared<MockJNI>();
  auto surface_factory =
      [](std::shared_ptr<AndroidContext> android_context,
         std::shared_ptr<PlatformViewAndroidJNI> jni_facade) {
        return std::make_unique<MockAndroidSurface>(/*id=*/123);
      };
  auto layer_1 = pool->GetLayer(gr_context_1.get(), android_context, mock_jni,
                                surface_factory);

  pool->RecycleLayers();

  auto gr_context_2 = GrContext::MakeMock(nullptr);
  auto layer_2 = pool->GetLayer(gr_context_2.get(), android_context, mock_jni,
                                surface_factory);
  ASSERT_NE(nullptr, layer_1);
  ASSERT_EQ(layer_1, layer_2);
  ASSERT_EQ(gr_context_2.get(), layer_1->gr_context);
  ASSERT_EQ(gr_context_2.get(), layer_2->gr_context);
}

TEST(SurfacePool, GetLayer__AllocateTwoLayers) {
  auto pool = new SurfacePool();

  auto gr_context_1 = GrContext::MakeMock(nullptr);
  auto android_context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);
  auto mock_jni = std::make_shared<MockJNI>();
  auto surface_factory =
      [](std::shared_ptr<AndroidContext> android_context,
         std::shared_ptr<PlatformViewAndroidJNI> jni_facade) {
        return std::make_unique<MockAndroidSurface>(/*id=*/123);
      };
  auto layer_1 = pool->GetLayer(gr_context_1.get(), android_context, mock_jni,
                                surface_factory);
  auto layer_2 = pool->GetLayer(gr_context_1.get(), android_context, mock_jni,
                                surface_factory);

  ASSERT_NE(nullptr, layer_1);
  ASSERT_NE(nullptr, layer_2);
  ASSERT_NE(layer_1, layer_2);
}

}  // namespace testing
}  // namespace flutter
