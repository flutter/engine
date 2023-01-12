//
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// @flow
// @format
//

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_ANIMATION_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_ANIMATION_H_

#include <Foundation/NSObject.h>

// This simplified spring model is based off of a damped harmonic oscillator.
// See: https://en.wikipedia.org/wiki/Harmonic_oscillator#Damped_harmonic_oscillator
//
// This models the closed form of the second order differential equation which happens to match the
// algorithm used by CASpringAnimation, a QuartzCore (iOS) API that creates spring animations.
@interface SpringAnimation : NSObject

- (instancetype)initWithStiffness:(double)stiffness
                          damping:(double)damping
                             mass:(double)mass
                  initialVelocity:(double)initialVelocity;

- (double)curveFunction:(double)t;

@property(nonatomic, assign) double initialVelocity;
@property(nonatomic, assign) double zeta;
@property(nonatomic, assign) double omega0;
@property(nonatomic, assign) double omega1;
@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_SPRING_ANIMATION_H_
