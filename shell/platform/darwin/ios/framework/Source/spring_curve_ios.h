// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_CURVE_IOS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_CURVE_IOS_H_

#include <Foundation/NSObject.h>

// This simplified spring model is based off of a damped harmonic oscillator.
// See: https://en.wikipedia.org/wiki/Harmonic_oscillator#Damped_harmonic_oscillator
//
// This models the closed form of the second order differential equation which happens to match the
// algorithm used by CASpringAnimation, a QuartzCore (iOS) API that creates spring animations.
@interface FlutterKeyboardSpringCurve : NSObject

- (instancetype)initWithStiffness:(double)stiffness
                          damping:(double)damping
                             mass:(double)mass
                  initialVelocity:(double)initialVelocity;

- (double)curveFunc:(double)t;

@property(nonatomic, assign) double initialVelocity;
@property(nonatomic, assign) double zeta;
@property(nonatomic, assign) double omega0;
@property(nonatomic, assign) double omega1;
@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_CURVE_IOS_H_
