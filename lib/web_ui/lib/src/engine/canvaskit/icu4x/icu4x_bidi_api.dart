// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:js/js.dart';
import 'package:js/js_util.dart' as js_util;

/// Entrypoint into the ICU4X API.
late ICU4XBidi icu4xBidi;

@JS('window.ICU4XBidiPromise')
external Object get _windowICU4XBidiPromise;

Future<ICU4XBidi> get windowICU4XBidiFuture => js_util.promiseToFuture<ICU4XBidi>(_windowICU4XBidiPromise);

@JS()
@anonymous
@staticInterop
class ICU4XBidi {}

extension ICU4XExtension on ICU4XBidi {
  List<int> getBidiRegions(String text, int? defaultLevel) => js_util.callMethod<List<Object?>>(
        this,
        'getBidiRegions',
        <Object?>[text, defaultLevel],
      ).cast<int>();
}
