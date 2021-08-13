// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/platform_message_router.h"

#include <vector>

#import "flutter/shell/platform/darwin/common/buffer_conversions.h"

namespace flutter {

PlatformMessageRouter::PlatformMessageRouter() = default;

PlatformMessageRouter::~PlatformMessageRouter() = default;

void PlatformMessageRouter::HandlePlatformMessage(
    std::unique_ptr<flutter::PlatformMessage> message) const {
  fml::RefPtr<flutter::PlatformMessageResponse> completer = message->response();
  auto it = message_handlers_.find(message->channel());
  if (it != message_handlers_.end()) {
    FlutterBinaryMessageHandler handler = it->second;
    NSData* data = nil;
    if (message->hasData()) {
      data = ConvertMappingToNSData(message->releaseData());
    }
    handler(data, ^(NSData* reply) {
      if (completer) {
        if (reply) {
          completer->Complete(ConvertNSDataToMappingPtr(reply));
        } else {
          completer->CompleteEmpty();
        }
      }
    });
  } else {
    if (completer) {
      completer->CompleteEmpty();
    }
  }
}

std::unique_ptr<fml::Mapping> PlatformMessageRouter::HandleFFIPlatformMessage(
    std::unique_ptr<flutter::PlatformMessage> message) const {
  // This executes on the ui thread.
  std::scoped_lock lock(ffi_mutex_);
  std::unique_ptr<fml::Mapping> response;
  auto it = ffi_message_handlers_.find(message->channel());
  if (it != ffi_message_handlers_.end()) {
    FlutterFFIBinaryMessageHandler handler = it->second;
    NSData* data = nil;
    if (message->hasData()) {
      data = ConvertMappingToNSData(message->releaseData());
    }
    NSData* reply = handler(data);
    if (reply) {
      response = ConvertNSDataToMappingPtr(reply);
    }
  }
  return response;
}

void PlatformMessageRouter::SetMessageHandler(const std::string& channel,
                                              FlutterBinaryMessageHandler handler) {
  message_handlers_.erase(channel);
  if (handler) {
    message_handlers_[channel] =
        fml::ScopedBlock<FlutterBinaryMessageHandler>{handler, fml::OwnershipPolicy::Retain};
  }
}

void PlatformMessageRouter::SetFFIMessageHandler(const std::string& channel,
                                                 FlutterFFIBinaryMessageHandler handler) {
  // This executes on the platform thread.
  std::scoped_lock lock(ffi_mutex_);
  ffi_message_handlers_.erase(channel);
  if (handler) {
    ffi_message_handlers_[channel] =
        fml::ScopedBlock<FlutterFFIBinaryMessageHandler>{handler, fml::OwnershipPolicy::Retain};
  }
}

}  // namespace flutter
