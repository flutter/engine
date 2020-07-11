// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dart_api.h"
#include "runtime/dart_isolate.h"
#define FML_USED_ON_EMBEDDER

#include <memory>

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

  ASSERT_EQ(configuration.client(), &client);
  ASSERT_EQ(configuration.window().viewport_metrics().device_pixel_ratio, 1.0);
  ASSERT_EQ(configuration.window().viewport_metrics().physical_width, 0.0);
  ASSERT_EQ(configuration.window().viewport_metrics().physical_height, 0.0);
}

TEST(PlatformConfigurationTest, PlatformConfigurationWindowMetricsUpdate) {
  DummyPlatformConfigurationClient client;
  PlatformConfiguration configuration(&client);

  configuration.SetWindowMetrics({2.0, 10.0, 20.0});
  ASSERT_EQ(configuration.window().viewport_metrics().device_pixel_ratio, 2.0);
  ASSERT_EQ(configuration.window().viewport_metrics().physical_width, 10.0);
  ASSERT_EQ(configuration.window().viewport_metrics().physical_height, 20.0);
}

}  // namespace testing
}  // namespace flutter
