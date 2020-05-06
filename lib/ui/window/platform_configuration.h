// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_PLATFORM_CONFIGURATION_H_
#define FLUTTER_LIB_UI_WINDOW_PLATFORM_CONFIGURATION_H_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/fml/time/time_point.h"
#include "flutter/lib/ui/semantics/semantics_update.h"
#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/lib/ui/window/pointer_data_packet.h"
#include "flutter/lib/ui/window/screen.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/lib/ui/window/window.h"
#include "lib/ui/window/screen_metrics.h"
#include "third_party/tonic/dart_persistent_value.h"

namespace tonic {
class DartLibraryNatives;

// So tonic::ToDart<std::vector<int64_t>> returns List<int> instead of
// List<dynamic>.
template <>
struct DartListFactory<int64_t> {
  static Dart_Handle NewList(intptr_t length) {
    return Dart_NewListOf(Dart_CoreType_Int, length);
  }
};

}  // namespace tonic

namespace flutter {
class FontCollection;
class PlatformMessage;
class Scene;

// Must match the AccessibilityFeatureFlag enum in framework.
enum class AccessibilityFeatureFlag : int32_t {
  kAccessibleNavigation = 1 << 0,
  kInvertColors = 1 << 1,
  kDisableAnimations = 1 << 2,
  kBoldText = 1 << 3,
  kReduceMotion = 1 << 4,
  kHighContrast = 1 << 5,
};

class PlatformConfigurationClient {
 public:
  virtual std::string DefaultRouteName() = 0;
  virtual void ScheduleFrame() = 0;
  virtual void Render(Scene* scene) = 0;
  virtual void UpdateSemantics(SemanticsUpdate* update) = 0;
  virtual void HandlePlatformMessage(fml::RefPtr<PlatformMessage> message) = 0;
  virtual FontCollection& GetFontCollection() = 0;
  virtual void UpdateIsolateDescription(const std::string isolate_name,
                                        int64_t isolate_port) = 0;
  virtual void SetNeedsReportTimings(bool value) = 0;
  virtual std::shared_ptr<const fml::Mapping> GetPersistentIsolateData() = 0;
  virtual std::unique_ptr<std::vector<std::string>>
  ComputePlatformResolvedLocale(
      const std::vector<std::string>& supported_locale_data) = 0;

 protected:
  virtual ~PlatformConfigurationClient();
};

class PlatformConfiguration final {
 public:
  explicit PlatformConfiguration(PlatformConfigurationClient* client);

  ~PlatformConfiguration();

  PlatformConfigurationClient* client() const { return client_; }

  void DidCreateIsolate();
  void UpdateLocales(const std::vector<std::string>& locales);
  void UpdateUserSettingsData(const std::string& data);
  void UpdateLifecycleState(const std::string& data);
  void UpdateSemanticsEnabled(bool enabled);
  void UpdateAccessibilityFeatures(int32_t flags);
  void DispatchPlatformMessage(fml::RefPtr<PlatformMessage> message);
  void DispatchPointerDataPacket(const PointerDataPacket& packet);
  void DispatchSemanticsAction(int32_t id,
                               SemanticsAction action,
                               std::vector<uint8_t> args);
  void BeginFrame(fml::TimePoint frameTime);
  void ReportTimings(std::vector<int64_t> timings);

  void CompletePlatformMessageResponse(int response_id,
                                       std::vector<uint8_t> data);
  void CompletePlatformMessageEmptyResponse(int response_id);

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

  typedef std::unordered_map<int64_t, std::shared_ptr<Screen>> ScreenMap;
  typedef std::unordered_map<int64_t, std::shared_ptr<Window>> WindowMap;

  //----------------------------------------------------------------------------
  /// @brief Retrieves the Window with the window ID given by window_id.
  ///
  /// @param[in] window_id  The identifier for the Window to retrieve.
  ///
  /// @return a shared_ptr to the Window, or nullptr if none is found.
  std::shared_ptr<Window> window(int64_t window_id);

  //----------------------------------------------------------------------------
  /// @brief Retrieves the Screen with the screen ID given by window_id.
  ///
  /// @param[in] screen_id  The identifier for the screen to retrieve.
  ///
  /// @return A shared_ptr to the Screen, or nullptr if none is found.
  std::shared_ptr<Screen> screen(int64_t screen_id);

  //----------------------------------------------------------------------------
  /// @brief Sets the viewport metrics of all viewports that exist. Any windows
  ///        not in this list are assumed to no longer exist, and will be
  ///        removed.
  ///
  /// @param[in] windows The list of viewport metrics to replace the old list
  /// with.
  void SetWindowMetrics(const std::vector<ViewportMetrics>& viewport_metrics);

  //----------------------------------------------------------------------------
  /// @brief Retrieves the map of window id to window.
  ///
  /// @return a const reference to the map of windows that exist.
  const WindowMap& windows() { return windows_; }

  //----------------------------------------------------------------------------
  /// @brief Sets the screen metrics of all screens that exist. Any screens not
  ///        in this list are assumed to no longer exist, and will be removed.
  ///
  /// @param[in] windows The list of windows to replace the old list with.
  void SetScreenMetrics(const std::vector<ScreenMetrics>& screen_metrics);

  //----------------------------------------------------------------------------
  /// @brief Retrieves the map of screen id to screen.
  ///
  /// @return a const reference to the map of screens that exist.
  const ScreenMap& screens() { return screens_; }

 private:
  PlatformConfigurationClient* client_;
  tonic::DartPersistentValue library_;
  ViewportMetrics viewport_metrics_;

  WindowMap windows_;
  ScreenMap screens_;

  // We use id 0 to mean that no response is expected.
  int next_response_id_ = 1;
  std::unordered_map<int, fml::RefPtr<PlatformMessageResponse>>
      pending_responses_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_PLATFORM_CONFIGURATION_H_
