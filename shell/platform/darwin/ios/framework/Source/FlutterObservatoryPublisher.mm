// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#import "FlutterObservatoryPublisher.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/runtime/dart_service_isolate.h"

@implementation FlutterObservatoryPublisher {
  NSNetService* netService;
  int callbackHandle;
}

- (instancetype)init {
#if FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE && \
    FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_DYNAMIC_RELEASE
  self = [super init];
  NSAssert(self, @"Super must not return null on init.");

  netService = [NSNetService alloc];

  blink::DartServiceIsolate::ObservatoryServerStateCallback callback =
      fml::MakeCopyable([self](const std::string& uri) mutable {
        auto colonPosition = uri.find_last_of(":");
        auto portSubstring = uri.substr(colonPosition + 1);
        auto port = std::stoi(portSubstring);

        NSString* serviceName = [NSString stringWithFormat:@"%@@%@",
                                                           [[[NSBundle mainBundle] infoDictionary]
                                                               objectForKey:@"CFBundleIdentifier"],
                                                           [NSProcessInfo processInfo].hostName];
        [netService initWithDomain:@"local."
                              type:@"_dartobservatory._tcp."
                              name:serviceName
                              port:port];
        [netService setDelegate:self];
        [netService publish];
      });

  callbackHandle = blink::DartServiceIsolate::AddServerStatusCallback(callback);

  return self;
#endif  // release mode only
  return nil;
}

- (void)netServiceDidPublish:(NSNetService*)sender {
  FML_DLOG(INFO) << "FlutterObservatoryPublisher is ready!";
}

- (void)netService:(NSNetService*)sender didNotPublish:(NSDictionary*)errorDict {
  FML_DLOG(ERROR) << "Could not register as server for FlutterObservatoryPublisher. Check your "
                     "network settings and relaunch the application.";
}

- (void)dealloc {
#if FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE && \
    FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_DYNAMIC_RELEASE

  [netService stop];
  blink::DartServiceIsolate::RemoveServerStatusCallback(callbackHandle);

#endif  // release mode

  [super dealloc];
}

@end
