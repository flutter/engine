// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';
import 'dart:ui';

import 'scenario.dart';

/// Regular Accessibility Scenario
class AccessibilityScenario extends Scenario {
  /// Creates the Accessibility scenario.
  ///
  /// The [window] parameter must not be null.
  AccessibilityScenario(Window window)
      : assert(window != null),
        super(window);

  @override
  void onBeginFrame(Duration duration) {
    final SemanticsUpdateBuilder builder = SemanticsUpdateBuilder();
    print(window.physicalSize.width);
    print(window.physicalSize.height);
    builder.updateNode(
      id: 0,
      rect: const Rect.fromLTWH(
        0,
        0,
        300,
        400,
      ),
      childrenInTraversalOrder: Int32List.fromList(<int>[1, 2]),
      childrenInHitTestOrder: Int32List.fromList(<int>[1, 2]),
    );

     builder.updateNode(
      id: 1,
      rect: const Rect.fromLTWH(
        0,
        0,
        300,
        100,
      ),
      childrenInTraversalOrder: Int32List.fromList(<int>[3, 4]),
      childrenInHitTestOrder: Int32List.fromList(<int>[3, 4]),
    );

    builder.updateNode(
      id: 2,
      rect: const Rect.fromLTWH(
        0,
        100,
        300,
        300,
      ),
      label: 'item2 label',
      value: 'item2 value',
    );

    builder.updateNode(
      id: 3,
      rect: const Rect.fromLTWH(
        0,
        0,
        50,
        50,
      ),
      label: 'item3 label',
      value: 'item3 value',
    );

    builder.updateNode(
      id: 4,
      rect: const Rect.fromLTWH(
        50,
        0,
        50,
        50,
      ),
      label: 'item4 label',
      value: 'item4 value',
    );

    final SemanticsUpdate update = builder.build();
    window.updateSemantics(update);
    update.dispose();
  }
}
