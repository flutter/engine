// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#import "FlutterObservatoryPublisher.h"

#if FLUTTER_RELEASE

@implementation FlutterObservatoryPublisher
@end

#else  // FLUTTER_RELEASE

#import <TargetConditionals.h>
// NSNetService works fine on physical devices before iOS 13.2.
// However, it doesn't expose the services to regular mDNS
// queries on the Simulator or on iOS 13.2+ devices.
//
// When debugging issues with this implementation, the following is helpful:
//
// 1) Running `dns-sd -Z _dartobservatory`. This is a built-in macOS tool that
//    can find advertized observatories using this method. If dns-sd can't find
//    it, then the observatory is not getting advertized over any network
//    interface that the host machine has access to.
// 2) The Python zeroconf package. The dns-sd tool can sometimes see things
//    that aren't advertizing over a network interface - for example, simulators
//    using NSNetService has been observed using dns-sd, but doesn't show up in
//    the Python package (which is a high quality socket based implementation).
//    If that happens, this code should be tweaked such that it shows up in both
//    dns-sd's output and Python zeroconf's detection.
// 3) The Dart multicast_dns package, which is what Flutter uses to find the
//    port and auth code. If the advertizement shows up in dns-sd and Python
//    zeroconf but not multicast_dns, then it is a bug in multicast_dns.
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_13_0
#include <Network/Network.h>
#endif

#include <dns_sd.h>
#include <net/if.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/fml/task_runner.h"
#include "flutter/runtime/dart_service_isolate.h"

@protocol FlutterObservatoryPublisherDelegate
- (instancetype)initWithOwner:(FlutterObservatoryPublisher*)owner;
- (void)publishServiceProtocolPort:(NSString*)uri;
- (void)stopService;

@property(readonly) fml::scoped_nsobject<NSURL> url;
@end

@interface FlutterObservatoryPublisher ()
- (NSData*)createTxtData:(NSURL*)url;

@property(readonly) NSString* serviceName;
@property(readonly) fml::scoped_nsobject<NSObject<FlutterObservatoryPublisherDelegate>> delegate;

@end

API_AVAILABLE(ios(13.0))
@interface ObservatoryNWListenerDelegate : NSObject <FlutterObservatoryPublisherDelegate>
@end

@interface ObservatoryNSNetServiceDelegate
    : NSObject <FlutterObservatoryPublisherDelegate, NSNetServiceDelegate>
@end

@interface ObservatoryDNSServiceDelegate : NSObject <FlutterObservatoryPublisherDelegate>
@end

@implementation ObservatoryDNSServiceDelegate {
  fml::scoped_nsobject<FlutterObservatoryPublisher> _owner;
  DNSServiceRef _dnsServiceRef;
}

@synthesize url;

- (instancetype)initWithOwner:(FlutterObservatoryPublisher*)owner {
  self = [super init];
  NSAssert(self, @"Super must not return null on init.");
  _owner.reset([owner retain]);
  return self;
}

- (void)stopService {
  if (_dnsServiceRef) {
    DNSServiceRefDeallocate(_dnsServiceRef);
    _dnsServiceRef = NULL;
  }
}

- (void)publishServiceProtocolPort:(NSString*)uri {
  // uri comes in as something like 'http://127.0.0.1:XXXXX/' where XXXXX is the port
  // number.
  url.reset([[NSURL alloc] initWithString:uri]);

  DNSServiceFlags flags = kDNSServiceFlagsDefault;
#if TARGET_IPHONE_SIMULATOR
  // Simulator needs to use local loopback explicitly to work.
  uint32_t interfaceIndex = if_nametoindex("lo0");
#else   // TARGET_IPHONE_SIMULATOR
  // Physical devices need to request all interfaces.
  uint32_t interfaceIndex = 0;
#endif  // TARGET_IPHONE_SIMULATOR
  const char* registrationType = "_dartobservatory._tcp";
  const char* domain = "local.";  // default domain
  uint16_t port = [[url port] unsignedShortValue];

  NSData* txtData = [_owner createTxtData:url.get()];
  int err =
      DNSServiceRegister(&_dnsServiceRef, flags, interfaceIndex,
                         [_owner.get().serviceName UTF8String], registrationType, domain, NULL,
                         htons(port), txtData.length, txtData.bytes, dnsRegistrationCallback, NULL);

  if (err != 0) {
    FML_LOG(ERROR) << "Failed to register observatory port with mDNS.";
  } else {
    DNSServiceSetDispatchQueue(_dnsServiceRef, dispatch_get_main_queue());
  }
}

static void DNSSD_API dnsRegistrationCallback(DNSServiceRef sdRef,
                                              DNSServiceFlags flags,
                                              DNSServiceErrorType errorCode,
                                              const char* name,
                                              const char* regType,
                                              const char* domain,
                                              void* context) {
  if (errorCode == kDNSServiceErr_NoError) {
    FML_DLOG(INFO) << "FlutterObservatoryPublisher is ready!";
  } else {
    FML_LOG(ERROR) << "Could not register as server for FlutterObservatoryPublisher. Check your "
                      "network settings and relaunch the application.";
  }
}

@end

@implementation ObservatoryNWListenerDelegate {
  fml::scoped_nsobject<FlutterObservatoryPublisher> _owner;
  nw_listener_t _listener;
}

@synthesize url;

- (instancetype)initWithOwner:(FlutterObservatoryPublisher*)owner {
  self = [super init];
  NSAssert(self, @"Super must not return null on init.");
  _owner.reset([owner retain]);
  return self;
}

- (void)dealloc {
  if (_listener != nil) {
    nw_listener_cancel(_listener);
    nw_release(_listener);
  }
  [super dealloc];
}

- (void)stopService {
  if (_listener != nil) {
    nw_listener_cancel(_listener);
  }
  _listener = nil;
}

- (void)publishServiceProtocolPort:(NSString*)uri {
  url.reset([[NSURL alloc] initWithString:uri]);

  // Make sure previous any previous listener is cancelled.
  [self stopService];

  nw_advertise_descriptor_t descriptor = nw_advertise_descriptor_create_bonjour_service(
      NSBundle.mainBundle.bundleIdentifier.UTF8String, "_dartobservatory._tcp", NULL);
  if (descriptor == nil) {
    FML_LOG(ERROR) << "Failed to create Bonjour service _dartobservatory._tcp.";
    return;
  }

  // uri comes in as something like 'http://127.0.0.1:XXXXX/YYYYY/' where XXXXX is the port and
  // YYYYY is the auth code. number.
  nw_txt_record_t txtRecord = nw_txt_record_create_dictionary();
  NSString* path = [[url path] substringFromIndex:MIN(1, [url path].length)];
  NSData* pathData = [path dataUsingEncoding:NSUTF8StringEncoding];

  nw_txt_record_set_key(txtRecord, "authCode", (uint8_t*)pathData.bytes, pathData.length);

  nw_advertise_descriptor_set_txt_record_object(descriptor, txtRecord);
  nw_release(txtRecord);

  nw_parameters_t parameters = nw_parameters_create_secure_tcp(NW_PARAMETERS_DISABLE_PROTOCOL,
                                                               NW_PARAMETERS_DEFAULT_CONFIGURATION);

  if (parameters == nil) {
    FML_LOG(ERROR) << "Failed to create mDNS networking parameters.";
    return;
  }
  nw_parameters_set_reuse_local_address(parameters, true);

#if TARGET_IPHONE_SIMULATOR
  // Simulator needs to use local loopback explicitly to work.
  nw_parameters_set_required_interface_type(parameters, nw_interface_type_loopback);
#else   // TARGET_IPHONE_SIMULATOR
  // Physical devices need to request all interfaces.
#endif  // TARGET_IPHONE_SIMULATOR

  if ([url port] == nil) {
    _listener = nw_listener_create(parameters);
  } else {
    _listener = nw_listener_create_with_port([url port].stringValue.UTF8String, parameters);
  }
  nw_release(parameters);

  nw_listener_set_queue(_listener, dispatch_get_main_queue());
  nw_listener_set_advertise_descriptor(_listener, descriptor);
  nw_release(descriptor);

  nw_listener_set_state_changed_handler(
      _listener, ^(nw_listener_state_t state, _Nullable nw_error_t error) {
        switch (state) {
          case nw_listener_state_ready:
            FML_DLOG(INFO) << "FlutterObservatoryPublisher is ready!";
            break;
          case nw_listener_state_failed:
            FML_LOG(ERROR)
                << "Could not register as server for FlutterObservatoryPublisher. Check your "
                   "network settings and relaunch the application.";
            break;
          case nw_listener_state_cancelled:
            FML_DLOG(INFO) << "Cancelled FlutterObservatoryPublisher port publication";
            break;
          default:
            break;
        }
      });

  nw_listener_set_new_connection_handler(_listener, ^(nw_connection_t connection) {
    FML_DLOG(INFO) << "New connection detected";
  });

  nw_listener_start(_listener);
}

@end

@implementation ObservatoryNSNetServiceDelegate {
  fml::scoped_nsobject<FlutterObservatoryPublisher> _owner;
  fml::scoped_nsobject<NSNetService> _netService;
}

@synthesize url;

- (instancetype)initWithOwner:(FlutterObservatoryPublisher*)owner {
  self = [super init];
  NSAssert(self, @"Super must not return null on init.");
  _owner.reset([owner retain]);
  return self;
}

- (void)stopService {
  [_netService.get() stop];
  [_netService.get() setDelegate:nil];
}

- (void)publishServiceProtocolPort:(NSString*)uri {
  // uri comes in as something like 'http://127.0.0.1:XXXXX/' where XXXXX is the port
  // number.
  url.reset([[NSURL alloc] initWithString:uri]);

  NSNetService* netServiceTmp = [[NSNetService alloc] initWithDomain:@"local."
                                                                type:@"_dartobservatory._tcp."
                                                                name:_owner.get().serviceName
                                                                port:[[url port] intValue]];
  [netServiceTmp setTXTRecordData:[_owner createTxtData:url.get()]];
  _netService.reset(netServiceTmp);
  [_netService.get() setDelegate:self];
  [_netService.get() publish];
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
  flutter::DartServiceIsolate::CallbackHandle _callbackHandle;
  std::unique_ptr<fml::WeakPtrFactory<FlutterObservatoryPublisher>> _weakFactory;
}

- (NSURL*)url {
  return [_delegate.get().url autorelease];
}

- (instancetype)init {
  self = [super init];
  NSAssert(self, @"Super must not return null on init.");

  if (@available(iOS 13.0, *)) {
    _delegate.reset([[ObservatoryNWListenerDelegate alloc] initWithOwner:self]);
  } else if (@available(iOS 9.3, *)) {
    _delegate.reset([[ObservatoryDNSServiceDelegate alloc] initWithOwner:self]);
  } else {
    _delegate.reset([[ObservatoryNSNetServiceDelegate alloc] initWithOwner:self]);
  }
  _weakFactory = std::make_unique<fml::WeakPtrFactory<FlutterObservatoryPublisher>>(self);

  fml::MessageLoop::EnsureInitializedForCurrentThread();

  _callbackHandle = flutter::DartServiceIsolate::AddServerStatusCallback(
      [weak = _weakFactory->GetWeakPtr(),
       runner = fml::MessageLoop::GetCurrent().GetTaskRunner()](const std::string& uri) {
        if (!uri.empty()) {
          runner->PostTask([weak, uri]() {
            if (weak) {
              [[weak.get() delegate]
                  publishServiceProtocolPort:[NSString stringWithUTF8String:uri.c_str()]];
            }
          });
        }
      });

  return self;
}

- (NSString*)serviceName {
  return NSBundle.mainBundle.bundleIdentifier;
}

- (NSData*)createTxtData:(NSURL*)url {
  // Check to see if there's an authentication code. If there is, we'll provide
  // it as a txt record so flutter tools can establish a connection.
  NSString* path = [[url path] substringFromIndex:MIN(1, [[url path] length])];
  NSData* pathData = [path dataUsingEncoding:NSUTF8StringEncoding];
  NSDictionary<NSString*, NSData*>* txtDict = @{
    @"authCode" : pathData,
  };
  return [NSNetService dataFromTXTRecordDictionary:txtDict];
}

- (void)dealloc {
  [_delegate stopService];

  flutter::DartServiceIsolate::RemoveServerStatusCallback(std::move(_callbackHandle));
  [super dealloc];
}
@end

#endif  // FLUTTER_RELEASE
