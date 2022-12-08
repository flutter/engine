// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_CURVE_IOS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_CURVE_IOS_H_

#include <Foundation/NSObject.h>

@interface KeyboardSpringCurve : NSObject

- (instancetype)initWithStiffness:(double)stiffness
                          damping:(double)damping
                             mass:(double)mass
                  initialVelocity:(double)initialVelocity
                 settlingDuration:(double)settlingDuration;

- (double)curveFunc:(double)t;

@property(nonatomic) double initialVelocity;
@property(nonatomic) double settlingDuration;
@property(nonatomic) double dampingRatio;
@property(nonatomic) double omega;
@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_CURVE_IOS_H_
