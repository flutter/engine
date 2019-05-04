// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(dnfield): remove unused_element ignores when https://github.com/dart-lang/sdk/issues/35164 is resolved.

part of ui;

// A service protocol extension to schedule a frame to be rendered into the
// window.
Future<developer.ServiceExtensionResponse> _scheduleFrame(
    String method,
    Map<String, String> parameters
    ) async {
  // Schedule the frame.
  window.scheduleFrame();
  // Always succeed.
  return new developer.ServiceExtensionResponse.result(json.encode(<String, String>{
    'type': 'Success',
  }));
}

@pragma('vm:entry-point')
void _setupHooks() {  // ignore: unused_element
  assert(() {
    // In debug mode, register the schedule frame extension.
    developer.registerExtension('ext.ui.window.scheduleFrame', _scheduleFrame);
    return true;
  }());
}

/// Stub implementation. See docs in `../ui/`.
List<int> saveCompilationTrace() {
  throw UnimplementedError();
}
