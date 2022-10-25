// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io' as io;
import 'dart:typed_data' show ByteData, Uint8List;
import 'dart:ui' as ui;

// Signals a waiting latch in the native test.
void signal() native 'Signal';

// Signals a waiting latch in the native test, passing a boolean value.
void signalBoolValue(bool value) native 'SignalBoolValue';

// Signals a waiting latch in the native test, passing a string value.
void signalStringValue(String value) native 'SignalStringValue';

// Signals a waiting latch in the native test, which returns a value to the fixture.
bool signalBoolReturn() native 'SignalBoolReturn';

// Notify the native test that the first frame has been scheduled.
void notifyFirstFrameScheduled() native 'NotifyFirstFrameScheduled';

void main() {}

@pragma('vm:entry-point')
void hiPlatformChannels() {
  ui.channelBuffers.setListener('hi',
      (ByteData? data, ui.PlatformMessageResponseCallback callback) async {
    ui.PlatformDispatcher.instance.sendPlatformMessage('hi', data,
        (ByteData? reply) {
      ui.PlatformDispatcher.instance
          .sendPlatformMessage('hi', reply, (ByteData? reply) {});
    });
    callback(data);
  });
}

@pragma('vm:entry-point')
void alertPlatformChannel() async {
  // Serializers for data types are in the framework, so this will be hardcoded.
  Uint8List data = Uint8List.fromList([
    13, // _valueMap
    2, // Size
    // key: "type"
    7, // _valueString
    'type'.length,
    ...'type'.codeUnits,
    // value: "announce"
    7,
    'announce'.length,
    ...'announce'.codeUnits,
    // key: "data"
    7, // _valueString
    'data'.length,
    ...'data'.codeUnits,
    // value: map
    13, // _valueMap
    1, // Size
    // key: "message"
    7, // _valueString
    'message'.length,
    ...'message'.codeUnits,
    // value: ""
    7, // _valueString
    0
  ]);
  ByteData byteData = data.buffer.asByteData();

  final Completer<ByteData?> enabled = Completer<ByteData?>();
  ui.PlatformDispatcher.instance.sendPlatformMessage('semantics', ByteData(0), (ByteData? reply){
    enabled.complete(reply);
  });
  await enabled.future;

  ui.PlatformDispatcher.instance.sendPlatformMessage('flutter/accessibility', byteData, (ByteData? _){});
}

@pragma('vm:entry-point')
void customEntrypoint() {}

@pragma('vm:entry-point')
void verifyNativeFunction() {
  signal();
}

@pragma('vm:entry-point')
void verifyNativeFunctionWithParameters() {
  signalBoolValue(true);
}

@pragma('vm:entry-point')
void verifyNativeFunctionWithReturn() {
  bool value = signalBoolReturn();
  signalBoolValue(value);
}

@pragma('vm:entry-point')
void readPlatformExecutable() {
  signalStringValue(io.Platform.executable);
}

@pragma('vm:entry-point')
void drawHelloWorld() {
  ui.PlatformDispatcher.instance.onBeginFrame = (Duration duration) {
    final ui.ParagraphBuilder paragraphBuilder =
        ui.ParagraphBuilder(ui.ParagraphStyle())..addText('Hello world');
    final ui.Paragraph paragraph = paragraphBuilder.build();

    paragraph.layout(const ui.ParagraphConstraints(width: 800.0));

    final ui.PictureRecorder recorder = ui.PictureRecorder();
    final ui.Canvas canvas = ui.Canvas(recorder);

    canvas.drawParagraph(paragraph, ui.Offset.zero);

    final ui.Picture picture = recorder.endRecording();
    final ui.SceneBuilder sceneBuilder = ui.SceneBuilder()
      ..addPicture(ui.Offset.zero, picture)
      ..pop();

    ui.window.render(sceneBuilder.build());
  };

  ui.PlatformDispatcher.instance.scheduleFrame();
  notifyFirstFrameScheduled();
}
