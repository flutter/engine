// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_stub.dart' if (dart.library.ffi) 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart';

Picture drawPicture(void Function(Canvas) drawCommands) {
  final PictureRecorder recorder = PictureRecorder();
  final Canvas canvas = Canvas(recorder);
  drawCommands(canvas);
  return recorder.endRecording();
}

/// Draws the [Picture]. This is in preparation for a golden test.
Future<void> drawPictureUsingCurrentRenderer(Picture picture) async {
  final SceneBuilder sb = SceneBuilder();
  sb.pushOffset(0, 0);
  sb.addPicture(Offset.zero, picture);
  await renderer.renderScene(sb.build());
}

Future<void> waitForFallbackFontsToStabilize() async {
  if (!isCanvasKit) {
    return;
  }

  // Fallback fonts start downloading as a post-frame callback.
  CanvasKitRenderer.instance.rasterizer.debugRunPostFrameCallbacks();
  // Font downloading begins asynchronously so we inject a timer before checking the download queue.
  await Future<void>.delayed(Duration.zero);
  while (notoDownloadQueue.isPending ||
      notoDownloadQueue.downloader.debugActiveDownloadCount > 0) {
    await notoDownloadQueue.debugWhenIdle();
    await notoDownloadQueue.downloader.debugWhenIdle();
    CanvasKitRenderer.instance.rasterizer.debugRunPostFrameCallbacks();
    // Dummy timer for the same reason as above.
    await Future<void>.delayed(Duration.zero);
  }
}

/// Returns [true] if this test is running in the CanvasKit renderer.
bool get isCanvasKit => renderer is CanvasKitRenderer;

/// Returns [true] if this test is running in the HTML renderer.
bool get isHtml => renderer is HtmlRenderer;

bool get isSkwasm => renderer is SkwasmRenderer;
