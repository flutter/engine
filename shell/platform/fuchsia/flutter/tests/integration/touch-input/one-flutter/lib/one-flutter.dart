// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(https://fxbug.dev/84961): Fix null safety and remove this language version.
// @dart=2.9

import 'dart:ui' as ui;

import 'package:fidl_fuchsia_ui_test_input/fidl_async.dart' as test_touch;
import 'package:flutter/material.dart';
import 'package:fuchsia_services/services.dart';
import 'package:zircon/zircon.dart';

void main() {
  print('Launching one-flutter view');
  MyApp app = MyApp();
  app.run();
}

class MyApp {


  void run() {
    childView.create(hitTestable, focusable, (ByteData reply) {
        // Set up window allbacks.
        window.onPointerDataPacket = (PointerDataPacket packet) {
          for (final data in packet.data) {
            if (data.change == PointerChange.down) {
              this._backgroundColor = _black;
            }
          }
          window.scheduleFrame();
        };
        window.onMetricsChanged = () {
          window.scheduleFrame();
        };
        window.onBeginFrame = (Duration duration) {
          this.beginFrame(duration);
        };

        // The child view should be attached to Scenic now.
        // Ready to build the scene.
        window.scheduleFrame();
      });
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
    for (final data in packet.data) {
      if (data.change == PointerChange.down) {
        this._backgroundColor = _yellow;
      }
    }
    window.scheduleFrame();
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key key}) : super(key: key);

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  // Each tap down event will bump up the counter, and we change the color.
  int _touchCounter = 0;

  final List<MaterialColor> _colors = <MaterialColor>[
    Colors.red,
    Colors.orange,
    Colors.yellow,
    Colors.green,
    Colors.blue,
    Colors.purple,
  ];

  final _responseListener = test_touch.TouchInputListenerProxy();

  _MyHomePageState() {
    Incoming.fromSvcPath()
      ..connectToService(_responseListener)
      ..close();

    // We inspect the lower-level data packets, instead of using the higher-level gesture library.
    WidgetsBinding.instance.window.onPointerDataPacket =
        (ui.PointerDataPacket packet) {
      // Record the time when the pointer event was received.
      int nowNanos = System.clockGetMonotonic();

      for (ui.PointerData data in packet.data) {
        print('Flutter received a pointer: ${data.toStringFull()}');
        if (data.change == ui.PointerChange.down) {
          setState(() {
            _touchCounter++; // Trigger color change on DOWN event.
          });
        }

        if (data.change == ui.PointerChange.down ||
            data.change == ui.PointerChange.move) {
          _respond(test_touch.TouchInputListenerReportTouchInputRequest(
              // Notify test that input was seen.
              localX: data.physicalX,
              localY: data.physicalY,
              timeReceived: nowNanos,
              componentName: 'one-flutter'));
        }
      }
    };
  }

  void _respond(
      test_touch.TouchInputListenerReportTouchInputRequest request) async {
    await _responseListener.reportTouchInput(request);
  }

  @override
  Widget build(BuildContext context) => Scaffold(
      appBar: AppBar(),
      body: Center(
          child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
            Container(
                width: 200,
                height: 200,
                decoration: BoxDecoration(
                    color: _colors[_touchCounter % _colors.length],
                    shape: BoxShape.rectangle))
          ])));
}
