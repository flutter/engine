// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';
import 'dart:ui';

import 'scenario.dart';

/// Regular Accessibility Scenario.
///
/// Builds an accessibility tree with element traversal order as 0, 1, 3, 4, 2.
class AccessibilityScenario extends Scenario {
  /// Creates the Accessibility scenario.
  ///
  /// The [window] parameter must not be null.
  AccessibilityScenario(Window window)
      : assert(window != null),
        super(window);

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder sceneBuilder = SceneBuilder();
    sceneBuilder.pushOffset(0, 0);
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    canvas.drawCircle(const Offset(50, 50), 50, Paint()..color = const Color(0xFFABCDEF));
    final Picture picture = recorder.endRecording();
    sceneBuilder.addPicture(const Offset(300, 300), picture);

    final Scene scene = sceneBuilder.build();
    window.render(scene);
    scene.dispose();

    final Float64List identityMatrix = Float64List(16);
    identityMatrix[0] = 1;
    identityMatrix[5] = 1;
    identityMatrix[10] = 1;
    identityMatrix[15] = 1;


    final SemanticsUpdateBuilder builder = SemanticsUpdateBuilder();
    print(window.physicalSize.width);
    print(window.physicalSize.height);
    builder.updateNode(
      id: 0,
      rect: const Rect.fromLTWH(
        10,
        0,
        300,
        400,
      ),
      label: 'item0 label',
      value: 'item0 value',
      childrenInTraversalOrder: Int32List.fromList(<int>[1, 2]),
      childrenInHitTestOrder: Int32List.fromList(<int>[1, 2]),
      transform: identityMatrix,
      platformViewId: -1,
    );

     builder.updateNode(
      id: 1,
      rect: const Rect.fromLTWH(
        10,
        0,
        300,
        100,
      ),
      label: 'item1 label',
      value: 'item1 value',
      childrenInTraversalOrder: Int32List.fromList(<int>[3, 4]),
      childrenInHitTestOrder: Int32List.fromList(<int>[3, 4]),
      transform: identityMatrix,
      platformViewId: -1,
    );

    builder.updateNode(
      id: 2,
      rect: const Rect.fromLTWH(
        10,
        105,
        300,
        300,
      ),
      label: 'item2 label',
      value: 'item2 value',
      transform: identityMatrix,
      platformViewId: -1,
    );

    builder.updateNode(
      id: 3,
      rect: const Rect.fromLTWH(
        10,
        0,
        50,
        50,
      ),
      label: 'item3 label',
      value: 'item3 value',
      transform: identityMatrix,
      platformViewId: -1,
    );

    builder.updateNode(
      id: 4,
      rect: const Rect.fromLTWH(
        70,
        0,
        50,
        50,
      ),
      label: 'item4 label',
      value: 'item4 value',
      transform: identityMatrix,
      platformViewId: -1,
    );
    final SemanticsUpdate update = builder.build();
    window.updateSemantics(update);
    update.dispose();
  }
}
