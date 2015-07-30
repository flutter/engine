// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:sky/animation/animation_performance.dart';
import 'package:sky/widgets/basic.dart';

abstract class AnimatedComponent extends StatefulComponent {

  AnimatedComponent({ Key key }) : super(key: key);

  void syncFields(AnimatedComponent source) { }

  void watch(AnimationPerformance performance) {
    performance.addListener(_performanceChanged);
  }

  void _performanceChanged() {
    if (mounted) {
      setState(() {
        // We don't actually have any state to change, per se,
        // we just know that we have in fact changed state.
      });
    }
  }

}
