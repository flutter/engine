// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/framework/Source/spring_curve_ios.h"

#include <Foundation/Foundation.h>

// Spring code adapted from React Native's Animation Library, see:
// https://github.com/facebook/react-native/blob/main/Libraries/Animated/animations/SpringAnimation.js
@implementation FlutterKeyboardSpringCurve
- (instancetype)initWithStiffness:(double)stiffness
                          damping:(double)damping
                             mass:(double)mass
                  initialVelocity:(double)initialVelocity {
  self = [super init];
  if (self) {
    _initialVelocity = initialVelocity;
    _zeta = damping / (2 * sqrt(stiffness * mass));  // damping ratio
    _omega0 = sqrt(stiffness / mass);                // undamped angular frequency of the oscillator
    _omega1 = self.omega0 * sqrt(1.0 - (self.zeta * self.zeta));  // exponential decay
  }
  return self;
}

- (double)curveFunc:(double)t {
  double y;
  if (self.zeta < 1) {
    // Under damped
    double envelope = exp(-self.zeta * self.omega0 * t);
    y = 1 - envelope * ((self.initialVelocity + self.zeta * self.omega0) / self.omega1 *
                            sin(self.omega1 * t) +
                        cos(self.omega1 * t));
  } else {
    // Critically damped spring
    double envelope = exp(-self.omega0 * t);
    y = 1 - envelope * (1 + (self.initialVelocity + self.omega0) * t);
  }

  return y;
}
@end
