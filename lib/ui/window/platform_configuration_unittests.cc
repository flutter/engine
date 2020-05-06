// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dart_api.h"
#include "runtime/dart_isolate.h"
#define FML_USED_ON_EMBEDDER

#include <memory>
#include <vector>

#include "flutter/lib/ui/window/platform_configuration.h"

#include "flutter/fml/mapping.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"
#include "lib/ui/text/font_collection.h"

namespace flutter {
namespace testing {

class DummyPlatformConfigurationClient : public PlatformConfigurationClient {
 public:
  DummyPlatformConfigurationClient() {
    std::vector<uint8_t> data;
    isolate_data_.reset(new ::fml::DataMapping(data));
  }
  virtual std::string DefaultRouteName() { return "TestRoute"; }
  virtual void ScheduleFrame() {}
  virtual void Render(Scene* scene) {}
  virtual void UpdateSemantics(SemanticsUpdate* update) {}
  virtual void HandlePlatformMessage(fml::RefPtr<PlatformMessage> message) {}
  virtual FontCollection& GetFontCollection() { return font_collection_; }
  virtual void UpdateIsolateDescription(const std::string isolate_name,
                                        int64_t isolate_port) {}
  virtual void SetNeedsReportTimings(bool value) {}
  virtual std::shared_ptr<const fml::Mapping> GetPersistentIsolateData() {
    return isolate_data_;
  }
  virtual std::unique_ptr<std::vector<std::string>>
  ComputePlatformResolvedLocale(
      const std::vector<std::string>& supported_locale_data) {
    return nullptr;
  };

 private:
  FontCollection font_collection_;
  std::shared_ptr<const fml::Mapping> isolate_data_;
};

TEST(PlatformConfigurationTest, PlatformConfigurationInitialization) {
  DummyPlatformConfigurationClient client;
  PlatformConfiguration configuration(&client);

  ASSERT_TRUE(configuration.windows().empty());
  ASSERT_TRUE(configuration.screens().empty());
  ASSERT_EQ(configuration.client(), &client);
}

TEST(PlatformConfigurationTest, PlatformConfigurationWindowMetricsUpdate) {
  DummyPlatformConfigurationClient client;
  PlatformConfiguration configuration(&client);

  configuration.SetWindowMetrics(
      {{1, 2.0, 1.0, 2.0, 10.0, 20.0}, {0, 1, 2, 3, 4, 5}});
  ASSERT_EQ(configuration.window(1)->viewport_metrics().view_id, 1);
  ASSERT_EQ(configuration.window(1)->viewport_metrics().device_pixel_ratio,
            2.0);
  ASSERT_EQ(configuration.window(1)->viewport_metrics().physical_left, 1.0);
  ASSERT_EQ(configuration.window(1)->viewport_metrics().physical_top, 2.0);
  ASSERT_EQ(configuration.window(1)->viewport_metrics().physical_width, 10.0);
  ASSERT_EQ(configuration.window(1)->viewport_metrics().physical_height, 20.0);

  ASSERT_EQ(configuration.window(0)->viewport_metrics().view_id, 0);
  ASSERT_EQ(configuration.window(0)->viewport_metrics().device_pixel_ratio,
            1.0);
  ASSERT_EQ(configuration.window(0)->viewport_metrics().physical_left, 2.0);
  ASSERT_EQ(configuration.window(0)->viewport_metrics().physical_top, 3.0);
  ASSERT_EQ(configuration.window(0)->viewport_metrics().physical_width, 4.0);
  ASSERT_EQ(configuration.window(0)->viewport_metrics().physical_height, 5.0);
}

TEST(PlatformConfigurationTest, PlatformConfigurationScreenMetricsUpdate) {
  DummyPlatformConfigurationClient client;
  PlatformConfiguration configuration(&client);

  configuration.SetScreenMetrics(
      {{1, 2.0, 1.0, 2.0, 10.0, 20.0}, {0, 1, 2, 3, 4, 5}});
  ASSERT_EQ(configuration.screen(1)->screen_metrics().screen_id, 1);
  ASSERT_EQ(configuration.screen(1)->screen_metrics().device_pixel_ratio, 2.0);
  ASSERT_EQ(configuration.screen(1)->screen_metrics().physical_left, 1.0);
  ASSERT_EQ(configuration.screen(1)->screen_metrics().physical_top, 2.0);
  ASSERT_EQ(configuration.screen(1)->screen_metrics().physical_width, 10.0);
  ASSERT_EQ(configuration.screen(1)->screen_metrics().physical_height, 20.0);

  ASSERT_EQ(configuration.screen(0)->screen_metrics().screen_id, 0);
  ASSERT_EQ(configuration.screen(0)->screen_metrics().device_pixel_ratio, 1.0);
  ASSERT_EQ(configuration.screen(0)->screen_metrics().physical_left, 2.0);
  ASSERT_EQ(configuration.screen(0)->screen_metrics().physical_top, 3.0);
  ASSERT_EQ(configuration.screen(0)->screen_metrics().physical_width, 4.0);
  ASSERT_EQ(configuration.screen(0)->screen_metrics().physical_height, 5.0);
}

}  // namespace testing
}  // namespace flutter
