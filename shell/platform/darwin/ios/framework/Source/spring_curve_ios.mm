// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/spring_curve_ios.h"

#include <Foundation/Foundation.h>

// This simplified spring model is based off of a damped harmonic oscillator.
// See: https://en.wikipedia.org/wiki/Harmonic_oscillator#Damped_harmonic_oscillator
//
// This models the closed form of the second order differential equation which happens to match the
// algorithm used by CASpringAnimation, a QuartzCore (iOS) API that creates spring animations.
//
// Spring code adapted from React Native's Animation Library, see:
// https://github.com/facebook/react-native/blob/main/Libraries/Animated/animations/SpringAnimation.js
@implementation KeyboardSpringCurve
- (instancetype)initWithStiffness:(double)stiffness
                          damping:(double)damping
                             mass:(double)mass
                  initialVelocity:(double)initialVelocity {
  self = [super init];
  if (self) {
    _stiffness = stiffness;
    _damping = damping;
    _mass = mass;
    _initialVelocity = initialVelocity;
  }
  return self;
}

- (double)curveFunc:(double)t {
  double zeta = self.damping / (2 * sqrt(self.stiffness * self.mass));  // damping ratio
  double omega0 = sqrt(self.stiffness / self.mass);  // undamped angular frequency of the oscillator
  double omega1 = omega0 * sqrt(1.0 - (zeta * zeta));  // exponential decay
  double v0 = self.initialVelocity;

  double y;
  if (zeta < 1) {
    // Under damped
    double envelope = exp(-zeta * omega0 * t);
    y = 1 - envelope * ((v0 + zeta * omega0) / omega1 * sin(omega1 * t) + cos(omega1 * t));
  } else {
    // Critically damped spring
    double envelope = exp(-omega0 * t);
    y = 1 - envelope * (1 + (v0 + omega0) * t);
  }

  return y;
}
@end
