// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';
import 'dart:ui';

import 'package:args/args.dart';
import 'package:fidl_fuchsia_ui_test_input/fidl_async.dart' as test_touch;
import 'package:fuchsia_services/services.dart';
import 'package:zircon/zircon.dart';

void main(List<String> args) {
  print('child-view: starting');

  TestApp app = TestApp();
  app.run();
}

class TestApp {
  static const _yellow = Color.fromARGB(255, 255, 255, 0);
  static const _pink = Color.fromARGB(255, 255, 0, 255);

  Color _backgroundColor = _pink;

  final _responseListener = test_touch.TouchInputListenerProxy();

  void run() {
    window.onPointerDataPacket = (PointerDataPacket packet) {
      this.pointerDataPacket(packet);
    };
    window.onMetricsChanged = () {
      window.scheduleFrame();
    };
    window.onBeginFrame = (Duration duration) {
      this.beginFrame(duration);
    };

    window.scheduleFrame();
  }

  void beginFrame(Duration duration) {
    final pixelRatio = window.devicePixelRatio;
    final size = window.physicalSize / pixelRatio;
    final physicalBounds = Offset.zero & size * pixelRatio;
    final recorder = PictureRecorder();
    final canvas = Canvas(recorder, physicalBounds);
    canvas.scale(pixelRatio, pixelRatio);
    final paint = Paint()..color = this._backgroundColor;
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height), paint);
    final picture = recorder.endRecording();
    final sceneBuilder = SceneBuilder()
      ..pushClipRect(physicalBounds)
      ..addPicture(Offset.zero, picture)
      ..pop();
    window.render(sceneBuilder.build());
  }

  void pointerDataPacket(PointerDataPacket packet) {
    int nowNanos = System.clockGetMonotonic();

    for (PointerData data in packet.data) {
      print('child-view received tap: ${data.toStringFull()}');

      if (data.change == PointerChange.down) {
        this._backgroundColor = _yellow;
      }

      if (data.change == PointerChange.down || data.change == PointerChange.move) {
        Incoming.fromSvcPath()
          ..connectToService(_responseListener)
          ..close();

        _respond(test_touch.TouchInputListenerReportTouchInputRequest(
          localX: data.physicalX,
          localY: data.physicalY,
          timeReceived: nowNanos,
          componentName: 'child-view',
        ));
      }
    }

    window.scheduleFrame();
  }

  void _respond(test_touch.TouchInputListenerReportTouchInputRequest request) async {
    print('child-view reporting touch input to TouchInputListener');
    await _responseListener.reportTouchInput(request);
  }
}
