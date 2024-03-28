// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "flutter/shell/platform/windows/compositor_software.h"
#include "flutter/shell/platform/windows/flutter_windows_view.h"
#include "flutter/shell/platform/windows/testing/engine_modifier.h"
#include "flutter/shell/platform/windows/testing/flutter_windows_engine_builder.h"
#include "flutter/shell/platform/windows/testing/mock_window_binding_handler.h"
#include "flutter/shell/platform/windows/testing/windows_test.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

namespace {
using ::testing::Return;

class MockFlutterWindowsView : public FlutterWindowsView {
 public:
  MockFlutterWindowsView(FlutterWindowsEngine* engine,
                         std::unique_ptr<WindowBindingHandler> window)
      : FlutterWindowsView(kImplicitViewId, engine, std::move(window)) {}
  virtual ~MockFlutterWindowsView() = default;

  MOCK_METHOD(bool,
              PresentSoftwareBitmap,
              (const void* allocation, size_t row_bytes, size_t height),
              (override));
  MOCK_METHOD(bool, ClearSoftwareBitmap, (), (override));

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(MockFlutterWindowsView);
};

class CompositorSoftwareTest : public WindowsTest {
 public:
  CompositorSoftwareTest() = default;
  virtual ~CompositorSoftwareTest() = default;

 protected:
  FlutterWindowsEngine* engine() { return engine_.get(); }
  MockFlutterWindowsView* view() { return view_.get(); }

  void UseHeadlessEngine() {
    FlutterWindowsEngineBuilder builder{GetContext()};

    engine_ = builder.Build();
  }

  void UseEngineWithView() {
    FlutterWindowsEngineBuilder builder{GetContext()};

    auto window = std::make_unique<MockWindowBindingHandler>();
    EXPECT_CALL(*window.get(), SetView).Times(1);
    EXPECT_CALL(*window.get(), GetWindowHandle).WillRepeatedly(Return(nullptr));

    engine_ = builder.Build();
    view_ = std::make_unique<MockFlutterWindowsView>(engine_.get(),
                                                     std::move(window));

    EngineModifier modifier{engine_.get()};
    modifier.SetImplicitView(view_.get());
  }

 private:
  std::unique_ptr<FlutterWindowsEngine> engine_;
  std::unique_ptr<MockFlutterWindowsView> view_;

  FML_DISALLOW_COPY_AND_ASSIGN(CompositorSoftwareTest);
};

}  // namespace

TEST_F(CompositorSoftwareTest, CreateBackingStore) {
  UseHeadlessEngine();

  auto compositor = CompositorSoftware{engine()};

  FlutterBackingStoreConfig config = {};
  FlutterBackingStore backing_store = {};

  ASSERT_TRUE(compositor.CreateBackingStore(config, &backing_store));
  ASSERT_TRUE(compositor.CollectBackingStore(&backing_store));
}

TEST_F(CompositorSoftwareTest, Present) {
  UseEngineWithView();

  auto compositor = CompositorSoftware{engine()};

  FlutterBackingStoreConfig config = {};
  FlutterBackingStore backing_store = {};

  ASSERT_TRUE(compositor.CreateBackingStore(config, &backing_store));

  FlutterLayer layer = {};
  layer.type = kFlutterLayerContentTypeBackingStore;
  layer.backing_store = &backing_store;
  const FlutterLayer* layer_ptr = &layer;

  EXPECT_CALL(*view(), PresentSoftwareBitmap).WillOnce(Return(true));
  EXPECT_TRUE(compositor.Present(view()->view_id(), &layer_ptr, 1));

  ASSERT_TRUE(compositor.CollectBackingStore(&backing_store));
}

TEST_F(CompositorSoftwareTest, PresentEmpty) {
  UseEngineWithView();

  auto compositor = CompositorSoftware{engine()};

  EXPECT_CALL(*view(), ClearSoftwareBitmap).WillOnce(Return(true));
  EXPECT_TRUE(compositor.Present(view()->view_id(), nullptr, 0));
}

TEST_F(CompositorSoftwareTest, UnknownViewIgnored) {
  UseEngineWithView();

  auto compositor = CompositorSoftware{engine()};

  FlutterBackingStoreConfig config = {};
  FlutterBackingStore backing_store = {};

  ASSERT_TRUE(compositor.CreateBackingStore(config, &backing_store));

  FlutterLayer layer = {};
  layer.type = kFlutterLayerContentTypeBackingStore;
  layer.backing_store = &backing_store;
  const FlutterLayer* layer_ptr = &layer;

  FlutterViewId unknown_view_id = 123;
  ASSERT_NE(view()->view_id(), unknown_view_id);
  ASSERT_EQ(engine()->view(unknown_view_id), nullptr);

  EXPECT_FALSE(compositor.Present(unknown_view_id, &layer_ptr, 1));

  ASSERT_TRUE(compositor.CollectBackingStore(&backing_store));
}

TEST_F(CompositorSoftwareTest, HeadlessPresentIgnored) {
  UseHeadlessEngine();

  auto compositor = CompositorSoftware{engine()};

  FlutterBackingStoreConfig config = {};
  FlutterBackingStore backing_store = {};

  ASSERT_TRUE(compositor.CreateBackingStore(config, &backing_store));

  FlutterLayer layer = {};
  layer.type = kFlutterLayerContentTypeBackingStore;
  layer.backing_store = &backing_store;
  const FlutterLayer* layer_ptr = &layer;

  EXPECT_FALSE(compositor.Present(kImplicitViewId, &layer_ptr, 1));

  ASSERT_TRUE(compositor.CollectBackingStore(&backing_store));
}

// Test compositing an upper layer on a base layer, each 2x2 pixels.
// Base layer has opaque pixel values:
// BLACK  RED
// GREEN  WHITE
// Overlay layer has pixel values:
// RED: 127   WHITE: 0
// BLUE: 127  BLACK: 255
TEST_F(CompositorSoftwareTest, PresentMultiLayers) {
  UseEngineWithView();

  auto compositor = CompositorSoftware{engine()};

  FlutterBackingStoreConfig config = {sizeof(config), {2, 2}};
  FlutterBackingStore backing_store0 = {sizeof(FlutterBackingStore), nullptr};
  FlutterBackingStore backing_store1 = {sizeof(FlutterBackingStore), nullptr};

  ASSERT_TRUE(compositor.CreateBackingStore(config, &backing_store0));
  ASSERT_TRUE(compositor.CreateBackingStore(config, &backing_store1));

  uint32_t pixels0[4] = {0xff000000, 0xff0000ff, 0xff00ff00, 0xffffffff};
  uint32_t pixels1[4] = {0x7f0000ff, 0x00ffffff, 0x7fff0000, 0xff000000};

  std::memcpy(const_cast<void*>(backing_store0.software.allocation), pixels0,
              sizeof(uint32_t) * 4);
  std::memcpy(const_cast<void*>(backing_store1.software.allocation), pixels1,
              sizeof(uint32_t) * 4);

  FlutterLayer layer0 = {};
  layer0.type = kFlutterLayerContentTypeBackingStore;
  layer0.backing_store = &backing_store0;
  layer0.offset = {0, 0};
  layer0.size = {2, 2};

  FlutterLayer layer1 = layer0;
  layer1.backing_store = &backing_store1;
  const FlutterLayer* layer_ptr[2] = {&layer0, &layer1};

  EXPECT_CALL(*view(), PresentSoftwareBitmap)
      .WillOnce([&](const void* allocation, size_t row_bytes, size_t height) {
        auto pixel_data = static_cast<const uint32_t*>(allocation);
        EXPECT_EQ(pixel_data[0], 0xff00007f);
        EXPECT_EQ(pixel_data[1], 0xff0000ff);
        EXPECT_EQ(pixel_data[2], 0xff7f8000);
        EXPECT_EQ(pixel_data[3], 0xff000000);
        return true;
      });
  EXPECT_TRUE(compositor.Present(view()->view_id(), layer_ptr, 2));

  ASSERT_TRUE(compositor.CollectBackingStore(&backing_store0));
  ASSERT_TRUE(compositor.CollectBackingStore(&backing_store1));
}

}  // namespace testing
}  // namespace flutter
