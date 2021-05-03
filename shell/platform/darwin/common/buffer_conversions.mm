// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/buffer_conversions.h"

/// A proxy object that behaves like NSData represented in a Mapping.
/// This isn't a subclass of NSData because NSData is in a class cluster.
@interface FlutterMappingData : NSObject
@end

@implementation FlutterMappingData {
  NSData* _data;
  std::unique_ptr<fml::Mapping> _mapping;
}

+ (NSData*)dataWithMapping:(std::unique_ptr<fml::Mapping>)mapping {
  return (NSData*)[[[FlutterMappingData alloc] initWithMapping:std::move(mapping)] autorelease];
}

- (instancetype)initWithMapping:(std::unique_ptr<fml::Mapping>)mapping {
  self = [super init];
  if (self) {
    const void* rawData = mapping->GetMapping();

    // Const cast is required because the NSData API requires it despite
    // guarentees that the buffer won't be deleted or modified.
    _data = [[NSData alloc] initWithBytesNoCopy:const_cast<void*>(rawData)
                                         length:mapping->GetSize()
                                   freeWhenDone:NO];

    _mapping = std::move(mapping);
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

- (BOOL)respondsToSelector:(SEL)aSelector {
  return [NSData instancesRespondToSelector:aSelector];
}
@end

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

- (BOOL)respondsToSelector:(SEL)aSelector {
  return [NSData instancesRespondToSelector:aSelector];
}
@end

namespace flutter {

std::vector<uint8_t> CopyNSDataToVector(NSData* data) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.bytes);
  return std::vector<uint8_t>(bytes, bytes + data.length);
}

std::unique_ptr<fml::Mapping> CopyNSDataToMapping(NSData* data) {
  return std::make_unique<fml::DataMapping>(CopyNSDataToVector(data));
}

NSData* ConvertMessageToNSData(fml::RefPtr<PlatformMessage> message) {
  return [FlutterMessageData dataWithMessage:std::move(message)];
}

NSData* ConvertMappingToNSData(std::unique_ptr<fml::Mapping> mapping) {
  return [FlutterMappingData dataWithMapping:std::move(mapping)];
}

}  // namespace flutter
