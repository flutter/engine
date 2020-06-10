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

class DummyWindowClient : public WindowClient {
 public:
  DummyWindowClient() {
    std::vector<uint8_t> data;
    isolate_data_.reset(new ::fml::DataMapping(data));
  }
  virtual std::string InitialRouteName() { return "TestRoute"; }
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

 private:
  FontCollection font_collection_;
  std::shared_ptr<const fml::Mapping> isolate_data_;
};

TEST(PlatformConfigurationTest, PlatformConfigurationInitialization) {
  DummyWindowClient client;
  PlatformConfiguration configuration(&client);

  ASSERT_NE(configuration.get_window(0), nullptr);
  ASSERT_NE(configuration.get_screen(0), nullptr);
  ASSERT_EQ(configuration.get_window(0)->window_id(), 0);
  ASSERT_EQ(configuration.get_window(0)->screen(), 0);
  ASSERT_EQ(configuration.get_screen(0)->screen_id(), 0);
}

}  // namespace testing
}  // namespace flutter
