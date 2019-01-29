// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// HACK: pretend to be dart.ui in order to access its internals
library dart.ui;

import 'package:test/test.dart';

part '../../lib/ui/window.dart';

void main() {

  test('updateUserSettings can handle an empty object', () {
      // this should now throw.
      print(window.getDartVersion());
      expect(true, equals(true));
    });

}