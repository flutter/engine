// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/embedder/FlutterTextureRegistrar.h"
#import "flutter/shell/platform/darwin/embedder/FlutterEmbedderAPIBridge.h"

@implementation FlutterTextureRegistrar {
  __weak id<FlutterTextureRegistrarDelegate> _delegate;

  __weak FlutterEmbedderAPIBridge* _embedderAPIBridge;

  // A mapping of textureID to internal FlutterExternalTexture wrapper.
  NSMutableDictionary<NSNumber*, FlutterExternalTexture*>* _textures;
}

- (instancetype)initWithDelegate:(id<FlutterTextureRegistrarDelegate>)delegate
               embedderAPIBridge:(FlutterEmbedderAPIBridge*)bridge {
  if (self = [super init]) {
    _delegate = delegate;
    _embedderAPIBridge = bridge;
    _textures = [[NSMutableDictionary alloc] init];
  }
  return self;
}

- (int64_t)registerTexture:(id<FlutterTexture>)texture {
  FlutterExternalTexture* externalTexture = [_delegate onRegisterTexture:texture];
  int64_t textureID = [externalTexture textureID];
  BOOL success = [_embedderAPIBridge registerTextureWithID:textureID];
  if (success) {
    _textures[@(textureID)] = externalTexture;
    return textureID;
  } else {
    NSLog(@"Unable to register the texture with id: %lld.", textureID);
    return 0;
  }
}

- (void)textureFrameAvailable:(int64_t)textureID {
  BOOL success = [_embedderAPIBridge markTextureFrameAvailable:textureID];
  if (!success) {
    NSLog(@"Unable to mark texture with id %lld as available.", textureID);
  }
}

- (void)unregisterTexture:(int64_t)textureID {
  bool success = [_embedderAPIBridge unregisterTextureWithID:textureID];
  if (success) {
    [_textures removeObjectForKey:@(textureID)];
  } else {
    NSLog(@"Unable to unregister texture with id: %lld.", textureID);
  }
}

- (FlutterExternalTexture*)getTextureWithID:(int64_t)textureID {
  return _textures[@(textureID)];
}

@end
