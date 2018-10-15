// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#import "FlutterObservatoryPublisher.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/fml/task_runner.h"
#include "flutter/runtime/dart_service_isolate.h"

#if FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_RELEASE || \
    FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DYNAMIC_RELEASE

@implementation FlutterObservatoryPublisher {
}

#else

@interface FlutterObservatoryPublisherDelegate : NSObject <NSNetServiceDelegate>
@end

@implementation FlutterObservatoryPublisherDelegate {
  std::unique_ptr<fml::WeakPtrFactory<FlutterObservatoryPublisherDelegate>> _weakFactory;
}

- (instancetype)init {
  self = [super init];
  NSAssert(self, @"Super must not return nil.");

  _weakFactory = std::make_unique<fml::WeakPtrFactory<FlutterObservatoryPublisherDelegate>>(self);

  return self;
}

- (fml::WeakPtr<FlutterObservatoryPublisherDelegate>)getWeakPtr {
  return _weakFactory->GetWeakPtr();
}

- (void)netServiceDidPublish:(NSNetService*)sender {
  FML_DLOG(INFO) << "FlutterObservatoryPublisher is ready!";
}

- (void)netService:(NSNetService*)sender didNotPublish:(NSDictionary*)errorDict {
  FML_LOG(ERROR) << "Could not register as server for FlutterObservatoryPublisher. Check your "
                    "network settings and relaunch the application.";
}

@end

@implementation FlutterObservatoryPublisher {
  fml::scoped_nsobject<NSNetService> _netService;
  fml::scoped_nsobject<FlutterObservatoryPublisherDelegate> _delegate;
  std::unique_ptr<blink::DartServiceIsolate::ObservatoryServerStateCallback> _callbackHandle;
}

- (instancetype)init {
  self = [super init];
  NSAssert(self, @"Super must not return null on init.");

  fml::MessageLoop::EnsureInitializedForCurrentThread();
  auto currentLoop = fml::MessageLoop::GetCurrent().GetTaskRunner();
  _delegate.reset([[FlutterObservatoryPublisherDelegate alloc] init]);
  auto delegateWeakPtr = [_delegate getWeakPtr];
  _netService.reset([NSNetService alloc]);

  blink::DartServiceIsolate::ObservatoryServerStateCallback callback =
      fml::MakeCopyable([delegate = std::move(delegateWeakPtr), netService = std::move(_netService),
                         loop = std::move(currentLoop)](const std::string& uri) mutable {
        // Bail out the delegate is no longer alive.
        if (!delegate) {
          return;
        }
        // Make sure to run back on the platform thread.
        fml::TaskRunner::RunNowOrPostTask(loop, [delegate = std::move(delegate),
                                                 netService = std::move(netService),
                                                 uri = std::move(uri)]() mutable {
          if (uri.empty()) {
            [netService stop];
            return;
          }
          // uri comes in as something like 'http://127.0.0.1:XXXXX/' where XXXXX is the port
          // number.
          NSURL* url = [[[NSURL alloc] initWithString:[NSString stringWithUTF8String:uri.c_str()]]
              autorelease];

          // DNS name has to be a max of 63 bytes.  Prefer to cut off the app name rather than
          // the device hostName. e.g. 'io.flutter.example@someones-iphone', or
          // 'ongAppNameBecauseThisCouldHappenAtSomePoint@somelongname-iphone'
          NSString* serviceName =
              [NSString stringWithFormat:@"%@@%@",
                                         [[[NSBundle mainBundle] infoDictionary]
                                             objectForKey:@"CFBundleIdentifier"],
                                         [NSProcessInfo processInfo].hostName];
          if ([serviceName length] > 63) {
            serviceName = [serviceName substringFromIndex:[serviceName length] - 63];
          }

          [netService initWithDomain:@"local."
                                type:@"_dartobservatory._tcp."
                                name:serviceName
                                port:[[url port] intValue]];

          [netService setDelegate:delegate.get()];
          [netService publish];
        });
      });

  _callbackHandle = blink::DartServiceIsolate::AddServerStatusCallback(callback);

  return self;
}

- (void)dealloc {
  [_netService stop];
  blink::DartServiceIsolate::RemoveServerStatusCallback(std::move(_callbackHandle));
  [super dealloc];
}

#endif  // FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE && FLUTTER_RUNTIME_MODE !=
        // FLUTTER_RUNTIME_MODE_DYNAMIC_RELEASE

@end
