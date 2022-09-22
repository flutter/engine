// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/embedder/FlutterEmbedderEngine.h"
#import "flutter/shell/platform/darwin/common/framework/Source/FlutterDartProject_Internal.h"

#include "flutter/shell/platform/embedder/embedder.h"

// Callbacks provided to the engine. See the called methods for documentation.
#pragma mark - Static methods provided to engine configuration

static void OnPlatformMessage(const FlutterPlatformMessage* message,
                              FlutterEmbedderEngine* engine) {
  [engine engineCallbackOnPlatformMessage:message];
}

@interface FlutterEmbedderEngine ()

- (void)engineCallbackOnPlatformMessage:(const FlutterPlatformMessage*)message;

@end

@implementation FlutterEmbedderEngine {
  // The embedding-API-level engine object.
  FLUTTER_API_SYMBOL(FlutterEngine) _engine;

  // Pointer to the Dart AOT snapshot and instruction data.
  _FlutterEngineAOTData* _aotData;

  // The project being run by this engine.
  FlutterDartProject* _project;
}

- (instancetype)initWithRenderer:(FlutterTextureRegistrar<FlutterRenderer>*)flutterRenderer {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");

  _project = project ?: [[FlutterDartProject alloc] init];

  _embedderAPI.struct_size = sizeof(FlutterEngineProcTable);
  FlutterEngineGetProcAddresses(&_embedderAPI);

  _renderer = flutterRenderer;
  [_renderer setEmbedderAPIDelegate:self];

  return self;
}

- (BOOL)prepareEmbedderAPI:(NSString*)entrypoint {
  // The first argument of argv is required to be the executable name.
  std::vector<const char*> argv = {[[self executableName] UTF8String]};
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
  // flutterArguments.update_semantics_node_callback = [](const FlutterSemanticsNode* node,
  //                                                      void* user_data) {
  //   FlutterEngine* engine = (__bridge FlutterEngine*)user_data;
  //   [engine updateSemanticsNode:node];
};
//   flutterArguments.update_semantics_custom_action_callback =
//       [](const FlutterSemanticsCustomAction* action, void* user_data) {
//         FlutterEngine* engine = (__bridge FlutterEngine*)user_data;
//         [engine updateSemanticsCustomActions:action];
//       };
//   flutterArguments.custom_dart_entrypoint = entrypoint.UTF8String;
//   flutterArguments.shutdown_dart_vm_when_done = true;
//   flutterArguments.dart_entrypoint_argc = dartEntrypointArgs.size();
//   flutterArguments.dart_entrypoint_argv = dartEntrypointArgs.data();
//   flutterArguments.root_isolate_create_callback = _project.rootIsolateCreateCallback;
//   flutterArguments.log_message_callback = [](const char* tag, const char* message,
//                                              void* user_data) {
//     if (tag && tag[0]) {
//       std::cout << tag << ": ";
//     }
//     std::cout << message << std::endl;
//   };

//   static size_t sTaskRunnerIdentifiers = 0;
//   const FlutterTaskRunnerDescription cocoa_task_runner_description = {
//       .struct_size = sizeof(FlutterTaskRunnerDescription),
//       .user_data = (void*)CFBridgingRetain(self),
//       .runs_task_on_current_thread_callback = [](void* user_data) -> bool {
//         return [[NSThread currentThread] isMainThread];
//       },
//       .post_task_callback = [](FlutterTask task, uint64_t target_time_nanos,
//                                void* user_data) -> void {
//         [((__bridge FlutterEngine*)(user_data)) postMainThreadTask:task
//                                            targetTimeInNanoseconds:target_time_nanos];
//       },
//       .identifier = ++sTaskRunnerIdentifiers,
//   };
//   const FlutterCustomTaskRunners custom_task_runners = {
//       .struct_size = sizeof(FlutterCustomTaskRunners),
//       .platform_task_runner = &cocoa_task_runner_description,
//   };
//   flutterArguments.custom_task_runners = &custom_task_runners;

//   [self loadAOTData:_project.assetsPath];
//   if (_aotData) {
//     flutterArguments.aot_data = _aotData;
//   }

//   flutterArguments.compositor = [self createFlutterCompositor];

//   flutterArguments.on_pre_engine_restart_callback = [](void* user_data) {
//     FlutterEngine* engine = (__bridge FlutterEngine*)user_data;
//     [engine engineCallbackOnPreEngineRestart];
//   };

//   FlutterRendererConfig rendererConfig = [_renderer createRendererConfig];
//   FlutterEngineResult result = _embedderEngine.embedderAPI.Initialize(
//       FLUTTER_ENGINE_VERSION, &rendererConfig, &flutterArguments, (__bridge void*)(self),
//       &_engine);
//   if (result != kSuccess) {
//     NSLog(@"Failed to initialize Flutter engine: error %d", result);
//     return NO;
//   }

//   result = _embedderEngine.embedderAPI.RunInitialized(_engine);
//   if (result != kSuccess) {
//     NSLog(@"Failed to run an initialized engine: error %d", result);
//     return NO;
//   }

return YES;
}

- (void)dealloc {
  [self shutDownEngine];
  if (_aotData) {
    _embedderAPI.CollectAOTData(_aotData);
  }
}

- (FlutterEngineProcTable&)embedderAPI {
  return _embedderEngine.embedderAPI;
}

/**
 * Note: Called from dealloc. Should not use accessors or other methods.
 */
- (void)shutDownEngine {
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

#pragma mark-- private methods

- (void)engineCallbackOnPlatformMessage:(const FlutterPlatformMessage*)message {
  NSData* messageData = nil;
  if (message->message_size > 0) {
    messageData = [NSData dataWithBytesNoCopy:(void*)message->message
                                       length:message->message_size
                                 freeWhenDone:NO];
  }
  NSString* channel = @(message->channel);
  __block const FlutterPlatformMessageResponseHandle* responseHandle = message->response_handle;

  FlutterBinaryReply binaryResponseHandler = ^(NSData* response) {
    if (responseHandle) {
      _embedderAPI.SendPlatformMessageResponse(self->_engine, responseHandle,
                                               static_cast<const uint8_t*>(response.bytes),
                                               response.length);
      responseHandle = NULL;
    } else {
      NSLog(@"Error: Message responses can be sent only once. Ignoring duplicate response "
             "on channel '%@'.",
            channel);
    }
  };

  FlutterEngineHandlerInfo* handlerInfo = _messengerHandlers[channel];
  if (handlerInfo) {
    handlerInfo.handler(messageData, binaryResponseHandler);
  } else {
    binaryResponseHandler(nil);
  }
}

@end
