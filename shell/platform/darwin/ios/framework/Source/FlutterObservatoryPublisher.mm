// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include <dns_sd.h>
#include <net/if.h>

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

@implementation FlutterObservatoryPublisher {
  DNSServiceRef _dnsServiceRef;
  bool _dnsServiceRefInitialized;

  blink::DartServiceIsolate::CallbackHandle _callbackHandle;
  std::unique_ptr<fml::WeakPtrFactory<FlutterObservatoryPublisher>> _weakFactory;
}

- (instancetype)init {
  self = [super init];
  NSAssert(self, @"Super must not return null on init.");

  _dnsServiceRefInitialized = false;
  _weakFactory = std::make_unique<fml::WeakPtrFactory<FlutterObservatoryPublisher>>(self);

  fml::MessageLoop::EnsureInitializedForCurrentThread();

  _callbackHandle = blink::DartServiceIsolate::AddServerStatusCallback(
      [weak = _weakFactory->GetWeakPtr(),
       runner = fml::MessageLoop::GetCurrent().GetTaskRunner()](const std::string& uri) {
        runner->PostTask([weak, uri]() {
          if (weak) {
            [weak.get() publishServiceProtocolPort:std::move(uri)];
          }
        });
      });

  return self;
}

- (void)dealloc {
  if (_dnsServiceRefInitialized) {
    DNSServiceRefDeallocate(_dnsServiceRef);
  }
  blink::DartServiceIsolate::RemoveServerStatusCallback(std::move(_callbackHandle));
  [super dealloc];
}

- (uint32_t)resolveInterface {
  // We want to use the loopback interface on the simulator, to force it to be available via
  // standard mDNS queries.
#if TARGET_IPHONE_SIMULATOR
  return if_nametoindex("lo0");
#else   // TARGET_IPHONE_SIMULATOR
  return 0;
#endif  // TARGET_IPHONE_SIMULATOR
}

- (void)publishServiceProtocolPort:(std::string)uri {
  if (_dnsServiceRefInitialized) {
    DNSServiceRefDeallocate(_dnsServiceRef);
  }
  if (uri.empty()) {
    return;
  }
  // uri comes in as something like 'http://127.0.0.1:XXXXX/' where XXXXX is the port
  // number.
  NSURL* url =
      [[[NSURL alloc] initWithString:[NSString stringWithUTF8String:uri.c_str()]] autorelease];

  NSString* serviceName =
      [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"];

  DNSServiceFlags flags = kDNSServiceFlagsDefault;
  uint32_t interfaceIndex = [self resolveInterface];
  const char* registrationType = "_dartobservatory._tcp";
  const char* domain = "local.";  // default domain
  uint16_t port = [[url port] intValue];

  int err = DNSServiceRegister(&_dnsServiceRef, flags, interfaceIndex, [serviceName UTF8String],
                               registrationType, domain, NULL, htons(port), 0, NULL,
                               registrationCallback, NULL);

  if (err != 0) {
    FML_LOG(ERROR) << "Failed to register observatory port with mDNS.";
  } else {
    DNSServiceProcessResult(_dnsServiceRef);
  }

  _dnsServiceRefInitialized = err == 0;
}

static void DNSSD_API registrationCallback(DNSServiceRef sdRef,
                                           DNSServiceFlags flags,
                                           DNSServiceErrorType errorCode,
                                           const char* name,
                                           const char* regType,
                                           const char* domain,
                                           void* context) {
  if (errorCode == kDNSServiceErr_NoError) {
    FML_LOG(ERROR) << "FlutterObservatoryPublisher is ready!";
  } else {
    FML_LOG(ERROR) << "Could not register as server for FlutterObservatoryPublisher. Check your "
                      "network settings and relaunch the application.";
  }
}

#endif  // FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE && FLUTTER_RUNTIME_MODE !=
        // FLUTTER_RUNTIME_MODE_DYNAMIC_RELEASE

@end
