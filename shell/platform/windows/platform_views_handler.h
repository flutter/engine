// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEWS_HANDLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEWS_HANDLER_H_

#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/encodable_value.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/platform_view_factory2.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "flutter/shell/platform/windows/window_binding_handler.h"

namespace flutter {

// Handler for PlatformViews
class PlatformViewsHandler {
 public:
  explicit PlatformViewsHandler(flutter::BinaryMessenger* messenger, PlatformWindow window);

  bool RegisterViewFactory(std::string view_type, PlatformViewFactory2* factory);

 private:
  // Called when a method is called on |channel_|;
  void HandleMethodCall(
      const flutter::MethodCall<EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<EncodableValue>> result);

  void HandleCreate(
      const EncodableMap& arguments,
      std::unique_ptr<flutter::MethodResult<EncodableValue>>& result);

  void HandleOffset(
      const EncodableMap& arguments,
      std::unique_ptr<flutter::MethodResult<EncodableValue>>& result);

  void HandleResize(
      const EncodableMap& arguments,
      std::unique_ptr<flutter::MethodResult<EncodableValue>>& result);

  void HandleDispose(
      const EncodableMap& arguments,
      std::unique_ptr<flutter::MethodResult<EncodableValue>>& result);

  /**
   * Get the value corresponding to the key from the map
   * @tparam T the type of value
   * @param arguments the arguments map
   * @param key the key to get
   * @param out_value if get value from map successfully, the out_value is set
   * to the value
   * @return successful to set the out_value or failed to get key from map
   */
  template <typename T>
  static bool GetValueFromMap(const EncodableMap& arguments,
                              const char* key,
                              T& out_value) {
    auto iter = arguments.find(EncodableValue(std::string(key)));
    if (iter == arguments.end()) {
      return false;
    }
    if (std::holds_alternative<T>(iter->second)) {
      const auto& value = std::get<T>(iter->second);
      //    FML_LOG(ERROR)
      //        << "---- eggfly: PlatformViewsHandler::create end(), value="
      //        << value;
      out_value = value;
      return true;
    }
    return false;
  }

  PlatformWindow platform_window_;
  
  // The MethodChannel used for communication with the Flutter engine.
  std::unique_ptr<flutter::MethodChannel<EncodableValue>> channel_;

  std::map<std::string, PlatformViewFactory2*> platform_view_factories_;
  
  std::map<int32_t, std::unique_ptr<PlatformView2>> platform_views_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEWS_HANDLER_H_
