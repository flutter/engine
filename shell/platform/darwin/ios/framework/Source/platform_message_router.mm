// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/platform_message_router.h"

#include <vector>

#import "flutter/shell/platform/darwin/common/buffer_conversions.h"

/// A proxy object that behaves like NSData represented in a PlatformMessage.
/// This isn't a subclass of NSData because NSData is in a class cluster.
@interface FlutterMessageData : NSObject
@end

@implementation FlutterMessageData {
  NSData* _data;
  fml::RefPtr<flutter::PlatformMessage> _platformMessage;
}

+ (NSData*)dataWithMessage:(fml::RefPtr<flutter::PlatformMessage>)platformMessage {
  return (NSData*)[[[FlutterMessageData alloc] initWithMessage:std::move(platformMessage)]
      autorelease];
}

- (instancetype)initWithMessage:(fml::RefPtr<flutter::PlatformMessage>)platformMessage {
  self = [super init];
  if (self) {
    const void* rawData = platformMessage->data().data();

    // Const cast is required because the NSData API requires it despite
    // guarentees that the buffer won't be deleted or modified.
    _data = [[NSData alloc] initWithBytesNoCopy:const_cast<void*>(rawData)
                                         length:platformMessage->data().size()
                                   freeWhenDone:NO];

    _platformMessage = std::move(platformMessage);
  }
  return self;
}

- (void)dealloc {
  [_data release];
  [super dealloc];
}

- (id)forwardingTargetForSelector:(SEL)aSelector {
  return _data;
}

@end

namespace flutter {

PlatformMessageRouter::PlatformMessageRouter() = default;

PlatformMessageRouter::~PlatformMessageRouter() = default;

void PlatformMessageRouter::HandlePlatformMessage(
    fml::RefPtr<flutter::PlatformMessage> message) const {
  fml::RefPtr<flutter::PlatformMessageResponse> completer = message->response();
  auto it = message_handlers_.find(message->channel());
  if (it != message_handlers_.end()) {
    FlutterBinaryMessageHandler handler = it->second;
    NSData* data = nil;
    if (message->hasData()) {
      data = [FlutterMessageData dataWithMessage:std::move(message)];
    }
    handler(data, ^(NSData* reply) {
      if (completer) {
        if (reply) {
          completer->Complete(CopyNSDataToMapping(reply));
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

void PlatformMessageRouter::SetMessageHandler(const std::string& channel,
                                              FlutterBinaryMessageHandler handler) {
  message_handlers_.erase(channel);
  if (handler) {
    message_handlers_[channel] =
        fml::ScopedBlock<FlutterBinaryMessageHandler>{handler, fml::OwnershipPolicy::Retain};
  }
}

}  // namespace flutter
