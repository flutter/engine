// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "Util.h"

@implementation Util

+ (FlutterEngine*)runEngineWithScenario:(NSString*)scenario
                         withCompletion:(void (^)(void))engineRunCompletion {
  NSAssert([scenario length] != 0, @"You need to provide a scenario");
  FlutterEngine* engine = [[FlutterEngine alloc]
      initWithName:[NSString stringWithFormat:@"Test engine for %@", scenario]
           project:nil];
  [engine runWithEntrypoint:nil];
  [engine.binaryMessenger
      setMessageHandlerOnChannel:@"scenario_status"
            binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
              [engine.binaryMessenger
                  sendOnChannel:@"set_scenario"
                        message:[scenario dataUsingEncoding:NSUTF8StringEncoding]];
              if (engineRunCompletion != nil) {
                engineRunCompletion();
              }
            }];
  return engine;
}

@end
