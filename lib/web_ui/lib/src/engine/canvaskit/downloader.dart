// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:js/js.dart';

import '../configuration.dart';
import '../dom.dart';
import '../initialization.dart';
import 'canvaskit_api.dart';
import 'icu4x/icu4x_bidi_api.dart';


String get canvasKitBuildUrl => configuration.canvasKitBaseUrl + (kProfileMode ? 'profiling/' : '');
String get canvasKitJavaScriptBindingsUrl => '${canvasKitBuildUrl}canvaskit.js';
String canvasKitWasmModuleUrl(String canvasKitBase, String file) => canvasKitBase + file;

String get icu4xBidiBuildUrl => 'icu4x/';
String get icu4xBidiJavaScriptBindingsUrl => '${icu4xBidiBuildUrl}icu4x_bidi.js';

/// Download and initialize the CanvasKit module.
///
/// Downloads the CanvasKit JavaScript, then calls `CanvasKitInit` to download
/// and intialize the CanvasKit wasm.
Future<CanvasKit> downloadCanvasKit() async {
  if (windowFlutterCanvasKit != null) {
    return windowFlutterCanvasKit!;
  }
  return windowFlutterCanvasKit = await _downloadCanvasKit();
}

Future<CanvasKit> _downloadCanvasKit() async {
  await _loadScript(canvasKitJavaScriptBindingsUrl, scriptProcessor: patchCanvasKitModule);

  return CanvasKitInit(CanvasKitInitOptions(
    locateFile: allowInterop(
      (String file, String unusedBase) => canvasKitWasmModuleUrl(canvasKitBuildUrl, file),
    ),
  ));
}

typedef ScriptProcessor = void Function(DomHTMLScriptElement script);


Future<ICU4XBidi> downloadICU4XBidi() async {
  await _loadScript(icu4xBidiJavaScriptBindingsUrl);

  // Once the JS file is loaded, it immediately exposes a promise that resolves
  // to the ICU4XBidi object.
  return windowICU4XBidiFuture;
}

Future<void> _loadScript(String url, {ScriptProcessor? scriptProcessor}) {
  final DomHTMLScriptElement script = createDomHTMLScriptElement();
  script.src = url;

  final Completer<void> completer = Completer<void>();

  late final DomEventListener loadCallback;
  late final DomEventListener errorCallback;

  void loadEventHandler(DomEvent _) {
    completer.complete();
    script.removeEventListener('load', loadCallback);
    script.removeEventListener('error', errorCallback);
  }
  void errorEventHandler(DomEvent errorEvent) {
    completer.completeError(errorEvent);
    script.removeEventListener('load', loadCallback);
    script.removeEventListener('error', errorCallback);
  }

  loadCallback = allowInterop(loadEventHandler);
  errorCallback = allowInterop(errorEventHandler);

  script.addEventListener('load', loadCallback);
  script.addEventListener('error', errorCallback);

  if (scriptProcessor != null) {
    scriptProcessor(script);
  }

  domDocument.head!.appendChild(script);
  return completer.future;
}
