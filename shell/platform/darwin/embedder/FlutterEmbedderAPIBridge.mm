// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#import "flutter/shell/platform/darwin/common/framework/Source/FlutterBaseDartProject_Internal.h"
#import "flutter/shell/platform/darwin/embedder/FlutterEmbedderAPIBridge.h"
#import "flutter/shell/platform/darwin/embedder/FlutterTextureRegistrar.h"

#include "flutter/shell/platform/embedder/embedder.h"

/**
 * Constructs and returns a FlutterLocale struct corresponding to |locale|, which must outlive
 * the returned struct.
 */
static FlutterLocale FlutterLocaleFromNSLocale(NSLocale* locale) {
  FlutterLocale flutterLocale = {};
  flutterLocale.struct_size = sizeof(FlutterLocale);
  flutterLocale.language_code = [[locale objectForKey:NSLocaleLanguageCode] UTF8String];
  flutterLocale.country_code = [[locale objectForKey:NSLocaleCountryCode] UTF8String];
  flutterLocale.script_code = [[locale objectForKey:NSLocaleScriptCode] UTF8String];
  flutterLocale.variant_code = [[locale objectForKey:NSLocaleVariantCode] UTF8String];
  return flutterLocale;
}

const uint64_t kFlutterDefaultViewId = 0;

// Callbacks provided to the bridge. See the called methods for documentation.
#pragma mark - Static methods provided to engine configuration

static void OnPlatformMessage(const FlutterPlatformMessage* message,
                              FlutterEmbedderAPIBridge* bridge) {
  [bridge engineCallbackOnPlatformMessage:message];
}

// Records an active handler of the messenger (FlutterEngine) that listens to
// platform messages on a given channel.
@interface FlutterEngineHandlerInfo : NSObject

- (instancetype)initWithConnection:(NSNumber*)connection
                           handler:(FlutterBinaryMessageHandler)handler;

@property(nonatomic, readonly) FlutterBinaryMessageHandler handler;
@property(nonatomic, readonly) NSNumber* connection;

@end

@implementation FlutterEngineHandlerInfo
- (instancetype)initWithConnection:(NSNumber*)connection
                           handler:(FlutterBinaryMessageHandler)handler {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _connection = connection;
  _handler = handler;
  return self;
}

@end

@interface FlutterEmbedderAPIBridge ()

/**
 * A mutable array that holds one bool value that determines if responses to platform messages are
 * clear to execute. This value should be read or written only inside of a synchronized block and
 * will return `NO` after the FlutterEngine has been dealloc'd.
 */
@property(nonatomic, strong) NSMutableArray<NSNumber*>* isResponseValid;

@end

@implementation FlutterEmbedderAPIBridge {
  // // The embedding-API-level engine object.
  FLUTTER_API_SYMBOL(FlutterEngine) _engine;

  // Pointer to the Dart AOT snapshot and instruction data.
  _FlutterEngineAOTData* _aotData;

  // FlutterCompositor is copied and used in embedder.cc.
  FlutterCompositor _compositor;

  // A mapping of channel names to the registered information for those channels.
  NSMutableDictionary<NSString*, FlutterEngineHandlerInfo*>* _messengerHandlers;

  // A self-incremental integer to assign to newly assigned channels as
  // identification.
  FlutterBinaryMessengerConnection _currentMessengerConnection;
}

- (instancetype)initWithProject:(nullable FlutterBaseDartProject*)project
                      presenter:(NSObject<FlutterPresenter>*)presenter {
  self = [super init];
  _project = project ?: [[FlutterBaseDartProject alloc] init];
  _messengerHandlers = [[NSMutableDictionary alloc] init];
  _currentMessengerConnection = 1;

  _embedderAPI.struct_size = sizeof(FlutterEngineProcTable);
  FlutterEngineGetProcAddresses(&_embedderAPI);

  _renderer = [[FlutterRenderer alloc] initWithEmbedderAPIBridge:self presenter:presenter];

  _isResponseValid = [[NSMutableArray alloc] initWithCapacity:1];
  [_isResponseValid addObject:@YES];
  return self;
}

- (BOOL)running {
  return _engine != nullptr;
}

- (void)dealloc {
  @synchronized(_isResponseValid) {
    [_isResponseValid removeAllObjects];
    [_isResponseValid addObject:@NO];
  }
  [self shutDownBridge];
  if (_aotData) {
    _embedderAPI.CollectAOTData(_aotData);
  }
}

/**
 * Note: Called from dealloc. Should not use accessors or other methods.
 */
- (void)shutDownBridge {
  if (_engine == nullptr) {
    return;
  }

  FlutterEngineResult result = _embedderAPI.Deinitialize(_engine);
  if (result != kSuccess) {
    NSLog(@"Could not de-initialize the Flutter engine: error %d", result);
  }

  // Balancing release for the retain in the task runner dispatch table.
  CFRelease((CFTypeRef)self);

  result = _embedderAPI.Shutdown(_engine);
  if (result != kSuccess) {
    NSLog(@"Failed to shut down Flutter engine: error %d", result);
  }
  _engine = nullptr;
}

- (FlutterEngineResult)initializeEmbedderAPIAndRun:(NSString*)entrypoint {
  // The first argument of argv is required to be the executable name.
  std::vector<const char*> argv = {[self.executableName UTF8String]};
  std::vector<std::string> switches = _project.switches;
  std::transform(switches.begin(), switches.end(), std::back_inserter(argv),
                 [](const std::string& arg) -> const char* { return arg.c_str(); });

  std::vector<const char*> dartEntrypointArgs;
  for (NSString* argument in [_project dartEntrypointArguments]) {
    dartEntrypointArgs.push_back([argument UTF8String]);
  }

  FlutterProjectArgs flutterArguments = {};
  flutterArguments.struct_size = sizeof(FlutterProjectArgs);
  flutterArguments.assets_path = _project.assetsPath.UTF8String;
  flutterArguments.icu_data_path = _project.ICUDataPath.UTF8String;
  flutterArguments.command_line_argc = static_cast<int>(argv.size());
  flutterArguments.command_line_argv = argv.empty() ? nullptr : argv.data();
  flutterArguments.platform_message_callback = (FlutterPlatformMessageCallback)OnPlatformMessage;
  flutterArguments.update_semantics_callback = [](const FlutterSemanticsUpdate* update,
                                                  void* user_data) {
    FlutterEmbedderAPIBridge* bridge = (__bridge FlutterEmbedderAPIBridge*)user_data;
    [bridge updateSemantics:update];
  };
  flutterArguments.custom_dart_entrypoint = entrypoint.UTF8String;
  flutterArguments.shutdown_dart_vm_when_done = true;
  flutterArguments.dart_entrypoint_argc = dartEntrypointArgs.size();
  flutterArguments.dart_entrypoint_argv = dartEntrypointArgs.data();
  flutterArguments.root_isolate_create_callback = _project.rootIsolateCreateCallback;
  flutterArguments.log_message_callback = [](const char* tag, const char* message,
                                             void* user_data) {
    if (tag && tag[0]) {
      std::cout << tag << ": ";
    }
    std::cout << message << std::endl;
  };

  static size_t sTaskRunnerIdentifiers = 0;
  const FlutterTaskRunnerDescription cocoa_task_runner_description = {
      .struct_size = sizeof(FlutterTaskRunnerDescription),
      .user_data = (void*)CFBridgingRetain(self),
      .runs_task_on_current_thread_callback = [](void* user_data) -> bool {
        return [[NSThread currentThread] isMainThread];
      },
      .post_task_callback = [](FlutterTask task, uint64_t target_time_nanos,
                               void* user_data) -> void {
        [((__bridge FlutterEmbedderAPIBridge*)(user_data)) postMainThreadTask:task
                                                      targetTimeInNanoseconds:target_time_nanos];
      },
      .identifier = ++sTaskRunnerIdentifiers,
  };
  const FlutterCustomTaskRunners custom_task_runners = {
      .struct_size = sizeof(FlutterCustomTaskRunners),
      .platform_task_runner = &cocoa_task_runner_description,
  };
  flutterArguments.custom_task_runners = &custom_task_runners;

  [self loadAOTData:_project.assetsPath];
  if (_aotData) {
    flutterArguments.aot_data = _aotData;
  }

  flutterArguments.compositor = [self createFlutterCompositor];

  flutterArguments.on_pre_engine_restart_callback = [](void* user_data) {
    FlutterEmbedderAPIBridge* bridge = (__bridge FlutterEmbedderAPIBridge*)user_data;
    [bridge engineCallbackOnPreEngineRestart];
  };

  FlutterRendererConfig rendererConfig = [_renderer createRendererConfig];
  FlutterEngineResult result = _embedderAPI.Initialize(
      FLUTTER_ENGINE_VERSION, &rendererConfig, &flutterArguments, (__bridge void*)(self), &_engine);
  if (result != kSuccess) {
    return result;
  }
  result = _embedderAPI.RunInitialized(_engine);
  return result;
}

#pragma mark - FlutterTextureRegistrar

- (int64_t)registerTexture:(id<FlutterTexture>)texture {
  return [_renderer registerTexture:texture];
}

- (BOOL)registerTextureWithID:(int64_t)textureId {
  return _embedderAPI.RegisterExternalTexture(_engine, textureId) == kSuccess;
}

- (void)textureFrameAvailable:(int64_t)textureID {
  [_renderer textureFrameAvailable:textureID];
}

- (BOOL)markTextureFrameAvailable:(int64_t)textureID {
  return _embedderAPI.MarkExternalTextureFrameAvailable(_engine, textureID) == kSuccess;
}

- (void)unregisterTexture:(int64_t)textureID {
  [_renderer unregisterTexture:textureID];
}

- (BOOL)unregisterTextureWithID:(int64_t)textureID {
  return _embedderAPI.UnregisterExternalTexture(_engine, textureID) == kSuccess;
}

#pragma mark-- getters

- (nonnull NSString*)executableName {
  return [[[NSProcessInfo processInfo] arguments] firstObject] ?: @"Flutter";
}

- (FLUTTER_API_SYMBOL(FlutterEngine))engine {
  return _engine;
}

#pragma mark - FlutterBinaryMessenger

- (void)sendOnChannel:(nonnull NSString*)channel message:(nullable NSData*)message {
  [self sendOnChannel:channel message:message binaryReply:nil];
}

- (void)sendOnChannel:(NSString*)channel
              message:(NSData* _Nullable)message
          binaryReply:(FlutterBinaryReply _Nullable)callback {
  FlutterPlatformMessageResponseHandle* response_handle = nullptr;
  if (callback) {
    struct Captures {
      FlutterBinaryReply reply;
    };
    auto captures = std::make_unique<Captures>();
    captures->reply = callback;
    auto message_reply = [](const uint8_t* data, size_t data_size, void* user_data) {
      auto captures = reinterpret_cast<Captures*>(user_data);
      NSData* reply_data = nil;
      if (data != nullptr && data_size > 0) {
        reply_data = [NSData dataWithBytes:static_cast<const void*>(data) length:data_size];
      }
      captures->reply(reply_data);
      delete captures;
    };

    FlutterEngineResult create_result = _embedderAPI.PlatformMessageCreateResponseHandle(
        _engine, message_reply, captures.get(), &response_handle);
    if (create_result != kSuccess) {
      NSLog(@"Failed to create a FlutterPlatformMessageResponseHandle (%d)", create_result);
      return;
    }
    captures.release();
  }

  FlutterPlatformMessage platformMessage = {
      .struct_size = sizeof(FlutterPlatformMessage),
      .channel = [channel UTF8String],
      .message = static_cast<const uint8_t*>(message.bytes),
      .message_size = message.length,
      .response_handle = response_handle,
  };

  FlutterEngineResult message_result = _embedderAPI.SendPlatformMessage(_engine, &platformMessage);
  if (message_result != kSuccess) {
    NSLog(@"Failed to send message to Flutter engine on channel '%@' (%d).", channel,
          message_result);
  }

  if (response_handle != nullptr) {
    FlutterEngineResult release_result =
        _embedderAPI.PlatformMessageReleaseResponseHandle(_engine, response_handle);
    if (release_result != kSuccess) {
      NSLog(@"Failed to release the response handle (%d).", release_result);
    };
  }
}

- (FlutterBinaryMessengerConnection)setMessageHandlerOnChannel:(nonnull NSString*)channel
                                          binaryMessageHandler:
                                              (nullable FlutterBinaryMessageHandler)handler {
  _currentMessengerConnection += 1;
  _messengerHandlers[channel] =
      [[FlutterEngineHandlerInfo alloc] initWithConnection:@(_currentMessengerConnection)
                                                   handler:[handler copy]];
  return _currentMessengerConnection;
}

- (void)cleanUpConnection:(FlutterBinaryMessengerConnection)connection {
  // Find the _messengerHandlers that has the required connection, and record its
  // channel.
  NSString* foundChannel = nil;
  for (NSString* key in [_messengerHandlers allKeys]) {
    FlutterEngineHandlerInfo* handlerInfo = [_messengerHandlers objectForKey:key];
    if ([handlerInfo.connection isEqual:@(connection)]) {
      foundChannel = key;
      break;
    }
  }
  if (foundChannel) {
    [_messengerHandlers removeObjectForKey:foundChannel];
  }
}

#pragma mark-- Embedder api callbacks

- (void)loadAOTData:(NSString*)assetsDir {
  if (!_embedderAPI.RunsAOTCompiledDartCode()) {
    return;
  }

  BOOL isDirOut = false;  // required for NSFileManager fileExistsAtPath.
  NSFileManager* fileManager = [NSFileManager defaultManager];

  // This is the location where the test fixture places the snapshot file.
  // For applications built by Flutter tool, this is in "App.framework".
  NSString* elfPath = [NSString pathWithComponents:@[ assetsDir, @"app_elf_snapshot.so" ]];

  if (![fileManager fileExistsAtPath:elfPath isDirectory:&isDirOut]) {
    return;
  }

  FlutterEngineAOTDataSource source = {};
  source.type = kFlutterEngineAOTDataSourceTypeElfPath;
  source.elf_path = [elfPath cStringUsingEncoding:NSUTF8StringEncoding];

  auto result = _embedderAPI.CreateAOTData(&source, &_aotData);
  if (result != kSuccess) {
    NSLog(@"Failed to load AOT data from: %@", elfPath);
  }
}

- (void)runTaskOnEmbedder:(FlutterTask)task {
  if (_engine) {
    auto result = _embedderAPI.RunTask(_engine, &task);
    if (result != kSuccess) {
      NSLog(@"Could not post a task to the Flutter engine.");
    }
  }
}

- (void)postMainThreadTask:(FlutterTask)task targetTimeInNanoseconds:(uint64_t)targetTime {
  __weak FlutterEmbedderAPIBridge* weakSelf = self;
  auto worker = ^{
    [weakSelf runTaskOnEmbedder:task];
  };

  const auto engine_time = _embedderAPI.GetCurrentTime();
  if (targetTime <= engine_time) {
    dispatch_async(dispatch_get_main_queue(), worker);

  } else {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, targetTime - engine_time),
                   dispatch_get_main_queue(), worker);
  }
}

- (void)engineCallbackOnPlatformMessage:(const FlutterPlatformMessage*)message {
  NSData* messageData = nil;
  if (message->message_size > 0) {
    messageData = [NSData dataWithBytesNoCopy:(void*)message->message
                                       length:message->message_size
                                 freeWhenDone:NO];
  }
  NSString* channel = @(message->channel);
  __block const FlutterPlatformMessageResponseHandle* responseHandle = message->response_handle;
  __block FlutterEmbedderAPIBridge* weakSelf = self;
  NSMutableArray* isResponseValid = self.isResponseValid;
  FlutterEngineSendPlatformMessageResponseFnPtr sendPlatformMessageResponse =
      _embedderAPI.SendPlatformMessageResponse;
  FlutterBinaryReply binaryResponseHandler = ^(NSData* response) {
    @synchronized(isResponseValid) {
      if (![isResponseValid[0] boolValue]) {
        // Ignore, engine was killed.
        return;
      }
      if (responseHandle) {
        sendPlatformMessageResponse(weakSelf->_engine, responseHandle,
                                    static_cast<const uint8_t*>(response.bytes), response.length);
        responseHandle = NULL;
      } else {
        NSLog(@"Error: Message responses can be sent only once. Ignoring duplicate response "
               "on channel '%@'.",
              channel);
      }
    }
  };

  FlutterEngineHandlerInfo* handlerInfo = _messengerHandlers[channel];
  if (handlerInfo) {
    handlerInfo.handler(messageData, binaryResponseHandler);
  } else {
    binaryResponseHandler(nil);
  }
}

- (void)updateSemantics:(const FlutterSemanticsUpdate*)update {
  // TODO implement this:
  // iOS and MacOS's a11y bridges should confirm to a protocol where this method is the interface.
}

- (void)engineCallbackOnPreEngineRestart {
  // TODO implement this,
  // iOS and MacOS's engine should be the delegate
}

- (FlutterCompositor*)createFlutterCompositor {
  FML_DCHECK(self.delegate);
  _compositor = {};
  _compositor.struct_size = sizeof(FlutterCompositor);
  _compositor.user_data = (__bridge void*)(self.delegate);

  _compositor.create_backing_store_callback = [](const FlutterBackingStoreConfig* config,  //
                                                 FlutterBackingStore* backing_store_out,   //
                                                 void* user_data                           //
                                              ) {
    return [(__bridge NSObject<FlutterEmbedderAPIBridgeDelegate>*)user_data
        compositorCreatingBackingStore:config
                       backingStoreOut:backing_store_out];
  };

  _compositor.collect_backing_store_callback = [](const FlutterBackingStore* backing_store,  //
                                                  void* user_data                            //
                                               ) { return true; };

  _compositor.present_layers_callback = [](const FlutterLayer** layers,  //
                                           size_t layers_count,          //
                                           void* user_data               //
                                        ) {
    // TODO(dkwingsmt): This callback only supports single-view, therefore it
    // only operates on the default view. To support multi-view, we need a new
    // callback that also receives a view ID.
    return [(__bridge NSObject<FlutterEmbedderAPIBridgeDelegate>*)user_data
        compositorPresent:kFlutterDefaultViewId
                   layers:layers
              layersCount:layers_count];
  };

  _compositor.avoid_backing_store_cache = true;

  return &_compositor;
}

- (void)sendPointerEvent:(const FlutterPointerEvent&)event {
  _embedderAPI.SendPointerEvent(_engine, &event, 1);
}

- (void)updateSemanticsEnabled:(BOOL)enable {
  _embedderAPI.UpdateSemanticsEnabled(_engine, enable);
}

- (void)dispatchSemanticsAction:(FlutterSemanticsAction)action
                       toTarget:(uint16_t)target
                       withData:(const uint8_t*)data
                           size:(size_t)size {
  _embedderAPI.DispatchSemanticsAction(_engine, target, action, data, size);
}

- (void)sendUserLocales {
  if (![self running]) {
    return;
  }

  // Create a list of FlutterLocales corresponding to the preferred languages.
  NSMutableArray<NSLocale*>* locales = [NSMutableArray array];
  std::vector<FlutterLocale> flutterLocales;
  flutterLocales.reserve(locales.count);
  for (NSString* localeID in [NSLocale preferredLanguages]) {
    NSLocale* locale = [[NSLocale alloc] initWithLocaleIdentifier:localeID];
    [locales addObject:locale];
    flutterLocales.push_back(FlutterLocaleFromNSLocale(locale));
  }
  // Convert to a list of pointers, and send to the engine.
  std::vector<const FlutterLocale*> flutterLocaleList;
  flutterLocaleList.reserve(flutterLocales.size());
  std::transform(flutterLocales.begin(), flutterLocales.end(),
                 std::back_inserter(flutterLocaleList),
                 [](const auto& arg) -> const auto* { return &arg; });
  _embedderAPI.UpdateLocales(_engine, flutterLocaleList.data(), flutterLocaleList.size());
}

#pragma mark-- macOS only callbacks
- (void)notifyDisplayUpdate:(FlutterEngineDisplayId)mainDisplayID
                refreshRate:(double)refreshRate
                 updateType:(FlutterEngineDisplaysUpdateType)updateType {
  FlutterEngineDisplay display;
  display.struct_size = sizeof(display);
  display.display_id = mainDisplayID;
  display.refresh_rate = refreshRate;

  std::vector<FlutterEngineDisplay> displays = {display};
  _embedderAPI.NotifyDisplayUpdate(_engine, updateType, displays.data(), displays.size());
}

- (void)SendWindowMetricsEventWithWidth:(size_t)width
                                 height:(size_t)height
                             pixelRatio:(double)pixelRatio
                                   left:(size_t)left
                                    top:(size_t)top {
  const FlutterWindowMetricsEvent windowMetricsEvent = {
      .struct_size = sizeof(windowMetricsEvent),
      .width = width,
      .height = height,
      .pixel_ratio = pixelRatio,
      .left = left,
      .top = top,
  };
  _embedderAPI.SendWindowMetricsEvent(_engine, &windowMetricsEvent);
}

- (void)sendKeyEvent:(const FlutterKeyEvent&)event
            callback:(FlutterKeyEventCallback)callback
            userData:(void*)userData {
  _embedderAPI.SendKeyEvent(_engine, &event, callback, userData);
}

@end
