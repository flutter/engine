// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl_test_fuchsia_service_foo/fidl_async.dart';
import 'package:fuchsia_services/services.dart';
import 'package:test/test.dart';
import 'package:fidl_fuchsia_sys/fidl_async.dart';

const String server =
    'fuchsia-pkg://fuchsia.com/fuchsia-services-foo-test-server#meta/fuchsia-services-foo-test-server.cmx';

void main() {
  test('launching and connecting to the foo service', () async {
    final incoming = Incoming();
    final launchInfo = LaunchInfo(
        url: server, directoryRequest: incoming.request().passChannel());
    final launcherProxy = LauncherProxy();

    final svc = Incoming.fromSvcPath()..connectToService(launcherProxy);
    await launcherProxy.createComponent(
        launchInfo, ComponentControllerProxy().ctrl.request());
    launcherProxy.ctrl.close();
    await svc.close();

    final fooProxy = FooProxy();
    incoming.connectToService(fooProxy);

    final response = await fooProxy.echo('foo');
    expect(response, 'foo');
  });
}
