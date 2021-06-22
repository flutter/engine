// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fuchsia_services/services.dart'; // ignore: implementation_imports
import 'package:test/test.dart';

void main() {
  final context = ComponentContext.createAndServe();

  group('ComponentContext: ', () {
    test('createAndServe does not return null instance', () {
      expect(context, isNotNull);
    });

    test('createAndServe serves the outgoing directory', () {
      // If outgoing is already serving, it will throw another exception if
      // `.serveFromStartupInfo` is called again.
      expect(context.outgoing.serveFromStartupInfo, throwsException);
    });

    test('create throws an error when called twice', () {
      expect(() => ComponentContext.create(), throwsException);
    });

    test('createAndServe throws an error when called twice', () {
      expect(() => ComponentContext.createAndServe(), throwsException);
    });
  });
}
