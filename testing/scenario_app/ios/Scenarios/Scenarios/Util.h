// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>

@interface Util : NSObject

+ (FlutterEngine* _Nullable ) runEngineWithScenario:(nonnull NSString*)scenario withCompletion:(void (^_Nullable)(void))engineRunCompletion;

@end
