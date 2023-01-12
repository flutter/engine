//
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// @flow
// @format
//

#import "spring_animation.h"

#include <Foundation/Foundation.h>

// Spring code adapted from React Native's Animation Library, see:
// https://github.com/facebook/react-native/blob/main/Libraries/Animated/animations/SpringAnimation.js
@implementation SpringAnimation
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

- (double)curveFunction:(double)t {
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
