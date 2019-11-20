// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show Base64Codec;
import 'dart:typed_data';
import 'dart:ui';
import 'scenario.dart';

const String _kImageString =
    'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAACXBIWXMAAAcTAAAHEwHOIA8IAAAB0mlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS40LjAiPgogICA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiPgogICAgICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgICAgICAgICB4bWxuczpwaG90b3Nob3A9Imh0dHA6Ly9ucy5hZG9iZS5jb20vcGhvdG9zaG9wLzEuMC8iCiAgICAgICAgICAgIHhtbG5zOnRpZmY9Imh0dHA6Ly9ucy5hZG9iZS5jb20vdGlmZi8xLjAvIj4KICAgICAgICAgPHBob3Rvc2hvcDpDcmVkaXQ+wqkgR29vZ2xlPC9waG90b3Nob3A6Q3JlZGl0PgogICAgICAgICA8dGlmZjpPcmllbnRhdGlvbj4xPC90aWZmOk9yaWVudGF0aW9uPgogICAgICA8L3JkZjpEZXNjcmlwdGlvbj4KICAgPC9yZGY6UkRGPgo8L3g6eG1wbWV0YT4K43gerQAAAA1JREFUCB1jeOVs+h8ABd8CYkMBAJAAAAAASUVORK5CYII=';

/// A screen contains an image and a drawn circle.
class ImageScenario extends Scenario {
  Image _image;

  /// Creates the ImageScenario.
  ///
  /// The [window] parameter must not be null.
  ImageScenario(Window window)
      : assert(window != null),
        super(window);

  Future<void> _loadScreenAsync() async {
    final Codec codec = await instantiateImageCodec(
        _getImageData(),
        targetHeight: 100,
        targetWidth: 100);
    final FrameInfo frameInfo = await codec.getNextFrame();
    _image = frameInfo.image;
    final SceneBuilder sceneBuilder = SceneBuilder();
    sceneBuilder.pushOffset(0, 0);
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    canvas.drawCircle(
      const Offset(50, 50),
      50,
      Paint()..color = const Color(0xFFABCDEF),
    );
    canvas.drawImage(_image, const Offset(50, 50), Paint());
    final Picture picture = recorder.endRecording();
    sceneBuilder.addPicture(const Offset(10, 10), picture);

    // Build a placeholder semantics node so the XCUITest can be notified that the drawImage operation
    // is ready by looking for this semantic node.
    final Float64List identityMatrix = Float64List(16);
    identityMatrix[0] = 1;
    identityMatrix[5] = 1;
    identityMatrix[10] = 1;
    identityMatrix[15] = 1;
    final SemanticsUpdateBuilder semanticsBuilder = SemanticsUpdateBuilder();
    semanticsBuilder.updateNode(
      id: 0,
      rect: const Rect.fromLTWH(
        0,
        0,
        10,
        10,
      ),
      label: 'ready',
      value: 'ready',
      transform: identityMatrix,
      childrenInTraversalOrder: Int32List.fromList(<int>[1, 2]),
      childrenInHitTestOrder: Int32List.fromList(<int>[1, 2]),
      platformViewId: -1,
    );
    final SemanticsUpdate update = semanticsBuilder.build();
    window.updateSemantics(update);
    update.dispose();

    final Scene scene = sceneBuilder.build();
    window.render(scene);
    scene.dispose();
  }

  @override
  void onBeginFrame(Duration duration) {
    _loadScreenAsync();
  }

  Uint8List _getImageData() {
    return const Base64Codec().decode(_kImageString);
  }
}
