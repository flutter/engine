// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_views_handler.h"

#include <windows.h>
#include <winuser.h>

#include "flutter/fml/logging.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"

static constexpr char kChannelName[] = "flutter/platform_views";

static constexpr char kCreateMethod[] = "create";

static constexpr char kOffsetMethod[] = "offset";

static constexpr char kResizeMethod[] = "resize";

static constexpr char kTouchMethod[] = "touch";

static constexpr char kDisposeMethod[] = "dispose";

static constexpr char kBadArgumentError[] = "Bad Arguments";

static constexpr char kViewIdKey[] = "id";

static constexpr char kViewTypeKey[] = "viewType";

static constexpr char kWidthKey[] = "width";
static constexpr char kHeightKey[] = "height";
static constexpr char kDirectionKey[] = "direction";

static constexpr char kTopKey[] = "top";
static constexpr char kLeftKey[] = "left";

namespace flutter {

PlatformViewsHandler::PlatformViewsHandler(BinaryMessenger* messenger, PlatformWindow window)
    : platform_window_(window), channel_(std::make_unique<MethodChannel<EncodableValue>>(
          messenger,
          kChannelName,
          &StandardMethodCodec::GetInstance())) {
  channel_->SetMethodCallHandler(
      [this](const MethodCall<EncodableValue>& call,
             std::unique_ptr<MethodResult<EncodableValue>> result) {
        HandleMethodCall(call, std::move(result));
      });
}

bool PlatformViewsHandler::RegisterViewFactory(std::string view_type, PlatformViewFactory2* factory) {
  FML_LOG(WARNING) << "RegisterViewFactory()";
  if (platform_view_factories_.find(view_type) != platform_view_factories_.end())  {
      FML_LOG(WARNING) << "Register platform view factory failed, because view_type already registered, view_type=" << view_type;
      return false;
  }
  platform_view_factories_.emplace(view_type, factory);
  return true;
}

void PlatformViewsHandler::HandleMethodCall(
    const MethodCall<EncodableValue>& method_call,
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  const std::string& method = method_call.method_name();
  if (!method_call.arguments() || method_call.arguments()->IsNull()) {
    result->Error(kBadArgumentError, "Method invoked without args");
    return;
  }
  if (std::holds_alternative<EncodableMap>(*method_call.arguments())) {
    const auto& arguments = std::get<EncodableMap>(*method_call.arguments());
    if (method.compare(kCreateMethod) == 0) {
      HandleCreate(arguments, result);
    } else if (method.compare(kOffsetMethod) == 0) {
      HandleOffset(arguments, result);
    } else if (method.compare(kResizeMethod) == 0) {
      HandleResize(arguments, result);
    } else if (method.compare(kTouchMethod) == 0) {
      result->Success();
    } else if (method.compare(kDisposeMethod) == 0) {
      HandleDispose(arguments, result);
    } else {
      result->NotImplemented();
    }
  } else {
    result->NotImplemented();
  }
}

void PlatformViewsHandler::HandleCreate(
    const EncodableMap & arguments,
    std::unique_ptr<MethodResult<EncodableValue>>& result) {
  FML_LOG(ERROR) << "---- eggfly:  PlatformViewsHandler::HandleMethodCall()";
//  if (!method_call.arguments() || method_call.arguments()->IsNull()) {
//    result->Error(kBadArgumentError, "Method invoked without args");
//    return;
//  }




//  const auto& arguments = std::get<EncodableMap>(*method_call.arguments());

  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::create() arguments()";

  std::string view_type;
  if (!GetValueFromMap<std::string>(arguments, kViewTypeKey, view_type)) {
    result->Error("Argument error",
                  "Missing argument \"viewType\" in \"create\" "
                  "method on channel flutter/platform_views.");
    return;
  }

  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::create end(), viewType="
                 << view_type;


  int32_t view_id = 0;
  if(!GetValueFromMap<int32_t>(arguments, kViewIdKey, view_id)) {
    result->Error("Argument error",
                  "Missing argument \"id\" in \"create\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  
  int32_t direction = 0;
  // TODO: cast to enum
  if (!GetValueFromMap<int32_t>(arguments, kDirectionKey, direction)) {
    result->Error("Argument error",
                  "Missing argument \"direction\" in \"create\" "
                  "method on channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR)
      << "---- eggfly: PlatformViewsHandler::create end(), direction="
      << direction;

  // const auto& view_id = std::get<int>(view_id_iter->second);
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::create end(), view_id="
                 << view_id;

  double width = 0;
  GetValueFromMap<double>(arguments, kWidthKey, width);
  double height = 0;
  GetValueFromMap<double>(arguments, kHeightKey, height);

  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::create end(), width="
                 << width;

  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::create end(), height="
                 << height;


  auto factory_iter = platform_view_factories_.find(view_type);
  if (factory_iter == platform_view_factories_.end()) {
    FML_LOG(ERROR) << "Cannot create because the factory of the view_type was not registered, view_type="<<view_type;
    // TODO set result
    return;
  }
  PlatformViewFactory2 * factory = factory_iter->second;
  HWND top_window_handle = ::GetParent(platform_window_);
  HWND grand_father_window = ::GetParent(top_window_handle);
  RECT bounds;
  ::GetWindowRect(top_window_handle, &bounds);
  FML_LOG(ERROR) << "---- eggfly: bounds: " << bounds.left <<"," << bounds.top  <<","<< bounds.right <<"," << bounds.bottom;

  FML_LOG(ERROR) << "---- eggfly: grand_father_window="<<grand_father_window;
  std::unique_ptr<PlatformView2> platform_view = factory->Create(top_window_handle);
  HWND view = reinterpret_cast<HWND>(platform_view->GetHWND());
  ::SetWindowPos(view, nullptr, 0,0, width * 2, height * 2, SWP_NOZORDER | SWP_NOACTIVATE);
  platform_views_[view_id] = std::move(platform_view);
  FML_LOG(ERROR) << "---- eggfly: platform_views_.size="<<platform_views_.size();
  result->Success();
}

void PlatformViewsHandler::HandleOffset(
    const EncodableMap & arguments,
    std::unique_ptr<MethodResult<EncodableValue>>& result) {
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::Offset()";

  int32_t view_id = 0;
  if(!GetValueFromMap<int32_t>(arguments, kViewIdKey, view_id)) {
    result->Error("Argument error",
                  "Missing argument \"id\" in \"offset\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::offset(), view_id=" << view_id;
  
  double top = 0;
  if(!GetValueFromMap<double>(arguments, kTopKey, top)) {
    result->Error("Argument error",
                  "Missing argument \"top\" in \"offset\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::offset(), top=" << top;
  

  double left = 0;
  if(!GetValueFromMap<double>(arguments, kLeftKey, left)) {
    result->Error("Argument error",
                  "Missing argument \"left\" in \"offset\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::offset(), left=" << left;
  
  auto platform_view_iter = platform_views_.find(view_id);
  if (platform_view_iter == platform_views_.end()) {
    FML_LOG(ERROR) << "Error handling offset method bacause view not found, view_id=" << view_id;
    // TODO
    // result
  }
  auto& platform_view = platform_view_iter->second;
  HWND view = reinterpret_cast<HWND>(platform_view->GetHWND());
  
  RECT bounds1;
  GetClientRect(view, &bounds1);
  RECT bounds2;
  GetWindowRect(view, &bounds2);

  FML_LOG(ERROR) << "bounds1 " << bounds1.left <<"," << bounds1.top  <<","<< bounds1.right <<"," << bounds1.bottom;
  FML_LOG(ERROR) << "bounds2 " << bounds2.left <<"," << bounds2.top  <<","<< bounds2.right <<"," << bounds2.bottom;
  
  ::SetWindowPos(view, nullptr, left * 2, top * 2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
  result->Success();
}


void PlatformViewsHandler::HandleResize(
    const EncodableMap & arguments,
    std::unique_ptr<MethodResult<EncodableValue>>& result) {
  FML_LOG(ERROR) << "---- eggfly:  PlatformViewsHandler::Resize()";

  int32_t view_id = 0;
  if(!GetValueFromMap<int32_t>(arguments, kViewIdKey, view_id)) {
    result->Error("Argument error",
                  "Missing argument \"id\" in \"resize\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::Resize(), view_id=" << view_id;

  double width = 0;
  if(!GetValueFromMap<double>(arguments, kWidthKey, width)) {
    result->Error("Argument error",
                  "Missing argument \"width\" in \"resize\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::resize(), width=" << width;


  double height = 0;
  if(!GetValueFromMap<double>(arguments, kHeightKey, height)) {
    result->Error("Argument error",
                  "Missing argument \"height\" in \"resize\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::resize(), height=" << height;

  auto platform_view_iter = platform_views_.find(view_id);
  if (platform_view_iter == platform_views_.end()) {
    FML_LOG(ERROR) << "Error handling offset method bacause view not found, view_id=" << view_id;
    // TODO
    // result
  }
  auto& platform_view = platform_view_iter->second;
  HWND view = reinterpret_cast<HWND>(platform_view->GetHWND());
  // auto dpi = ::GetDpiForWindow(view);
  // FML_LOG(ERROR) << "---- eggfly: dpi=" << dpi;
  ::SetWindowPos(view, nullptr, 0, 0, width * 2, height * 2, SWP_NOMOVE | SWP_NOACTIVATE);
  
  EncodableValue map(EncodableMap{
      {EncodableValue("width"), EncodableValue(width)},
      {EncodableValue("height"), EncodableValue(height)},
  });

  result->Success(map);
}

void PlatformViewsHandler::HandleDispose(
    const EncodableMap & arguments,
    std::unique_ptr<MethodResult<EncodableValue>>& result) {
  FML_LOG(ERROR) << "---- eggfly:  PlatformViewsHandler::Dispose()";

  int32_t view_id = 0;
  if(!GetValueFromMap<int32_t>(arguments, kViewIdKey, view_id)) {
    result->Error("Argument error",
                  "Missing argument \"id\" in \"dispose\" method on "
                  "channel flutter/platform_views.");
    return;
  }
  FML_LOG(ERROR) << "---- eggfly: PlatformViewsHandler::Dispose(), view_id=" << view_id;

  result->Success();
}

}  // namespace flutter
