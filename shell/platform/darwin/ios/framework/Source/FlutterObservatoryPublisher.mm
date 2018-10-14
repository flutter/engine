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

  NSRunLoop* currentLoop = [NSRunLoop currentRunLoop];
  blink::DartServiceIsolate::ObservatoryServerStateCallback callback =
      fml::MakeCopyable([self, currentLoop](const std::string& uri) mutable {
        if (uri.empty()) {
          [netService stop];
          [netService release];
          return;
        }
        // uri comes in as something like 'http://127.0.0.1:XXXXX/' where XXXXX is the port number.
        auto colonPosition = uri.find_last_of(":");
        auto portSubstring = uri.substr(colonPosition + 1);
        auto port = std::stoi(portSubstring);

        // DNS name has to be a max of 63 bytes.  Prefer to cut off the app name rather than the
        // device hostName.
        // e.g. 'io.flutter.example@someones-iphone', or
        // 'ongAppNameBecauseThisCouldHappenAtSomePoint@somelongname-iphone'
        NSString* serviceName = [NSString stringWithFormat:@"%@@%@",
                                                           [[[NSBundle mainBundle] infoDictionary]
                                                               objectForKey:@"CFBundleIdentifier"],
                                                           [NSProcessInfo processInfo].hostName];
        if ([serviceName length] > 63) {
          serviceName = [serviceName substringFromIndex:[serviceName length] - 63];
        }

        netService = [[NSNetService alloc] initWithDomain:@"local."
                                                     type:@"_dartobservatory._tcp."
                                                     name:serviceName
                                                     port:port];
        // If we don't run in the platform loop, we won't get signalled properly later.
        [netService removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [netService scheduleInRunLoop:currentLoop forMode:NSDefaultRunLoopMode];

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
  FML_DLOG(FATAL) << "Could not register as server for FlutterObservatoryPublisher. Check your "
                     "network settings and relaunch the application.";
}

- (void)dealloc {
#if FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE && \
    FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_DYNAMIC_RELEASE

  [netService stop];
  [netService release];
  blink::DartServiceIsolate::RemoveServerStatusCallback(callbackHandle);

#endif  // release mode

  [super dealloc];
}

@end
