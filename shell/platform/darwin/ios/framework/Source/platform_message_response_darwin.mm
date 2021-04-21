// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/platform_message_response_darwin.h"

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

@end

namespace flutter {

PlatformMessageResponseDarwin::PlatformMessageResponseDarwin(
    PlatformMessageResponseCallback callback,
    fml::RefPtr<fml::TaskRunner> platform_task_runner)
    : callback_(callback, fml::OwnershipPolicy::Retain),
      platform_task_runner_(std::move(platform_task_runner)) {}

PlatformMessageResponseDarwin::~PlatformMessageResponseDarwin() = default;

void PlatformMessageResponseDarwin::Complete(std::unique_ptr<fml::Mapping> data) {
  fml::RefPtr<PlatformMessageResponseDarwin> self(this);
  platform_task_runner_->PostTask(fml::MakeCopyable([self, data = std::move(data)]() mutable {
    NSData* callbackData = [FlutterMappingData dataWithMapping:std::move(data)];
    self->callback_.get()(callbackData);
  }));
}

void PlatformMessageResponseDarwin::CompleteEmpty() {
  fml::RefPtr<PlatformMessageResponseDarwin> self(this);
  platform_task_runner_->PostTask(
      fml::MakeCopyable([self]() mutable { self->callback_.get()(nil); }));
}

}  // namespace flutter
