// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'platform_echo_mixin.dart';
import 'scenario.dart';

/// A blank page with a button that pops the page when tapped.
class PoppableScreenScenario extends Scenario with PlatformEchoMixin {
  /// Creates the PoppableScreenScenario.
  ///
  /// The [window] parameter must not be null.
  PoppableScreenScenario(Window window)
      : assert(window != null),
        super(window);

  // Rect for the pop button. Only defined once onMetricsChanged is called.
  Rect _buttonRect;

  /// Used to animate the red value in the color of the square.
  final _NumberSwinger<int> _r = _NumberSwinger<int>(0, 255);

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder builder = SceneBuilder();
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);

    canvas.drawRect(
      Rect.fromLTWH(0, 0, window.physicalSize.width, window.physicalSize.height),
      Paint()..color = Color.fromARGB(255, 0, 0, 0),
    );

    if (_buttonRect) {
      canvas.drawRect(
        _buttonRect,
        Paint()..color = Color.fromARGB(255, 255, 0, 0),
      );
    }
    final Picture picture = recorder.endRecording();

    builder.addPicture(Offset.zero, picture);
    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }

  @override
  void onDrawFrame() {
    window.scheduleFrame();
  }

  @override
  void onMetricsChanged() {
    _buttonRect = Rect.fromLTRB(
      window.physicalSize.width / 3,
      window.physicalSize.height * 2 / 5,
      window.physicalSize.width * 2 / 3,
      window.physicalSize.height * 3 / 5,
    );
  }

  @override
  void onPointerDataPacket(PointerDataPacket packet) {
    for (PointerData in packet.data) {

    }
  }
}
