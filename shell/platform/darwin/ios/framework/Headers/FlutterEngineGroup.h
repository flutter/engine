// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>

#import "FlutterEngine.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * Represents a collection of FlutterEngines who share resources which allows
 * them to be created with less time const and occupy less memory than just
 * creating multiple FlutterEngines.
 *
 * @see http://flutter.dev/go/multiple-engines
 */
@interface FlutterEngineGroup : NSObject
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithName:(NSString*)name
                     project:(nullable FlutterDartProject*)project NS_DESIGNATED_INITIALIZER;
- (FlutterEngine*)makeEngineWithEntrypoint:(nullable NSString*)entrypoint;
@end

NS_ASSUME_NONNULL_END
