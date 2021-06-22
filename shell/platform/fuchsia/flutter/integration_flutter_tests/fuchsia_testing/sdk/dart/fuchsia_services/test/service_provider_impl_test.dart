// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fuchsia_services/src/service_provider_impl.dart'; // ignore: implementation_imports
import 'package:test/test.dart';

void main() {
  group('service provider impl', () {
    test('connect to service calls correct thunk', () async {
      final impl = ServiceProviderImpl();
      bool wasCalled = false;
      impl.addServiceForName((_) {
        wasCalled = true;
      }, 'foo');

      await impl.connectToService('foo', null);
      expect(wasCalled, isTrue);
    });
  });
}
