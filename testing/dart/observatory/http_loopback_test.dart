// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:developer' as developer;
import 'dart:io';
import 'dart:isolate';

import 'package:litetest/litetest.dart';
import 'package:vm_service/vm_service.dart' as vms;
import 'package:vm_service/vm_service_io.dart';
import '../http_disallow_http_connections_test.dart';

void main() {
  test('IsLoopback called on last resort', () async {
    final developer.ServiceProtocolInfo info = await developer.Service.getInfo();

    if (info.serverUri == null) {
      fail('This test must not be run with --disable-observatory.');
    }

    final vms.VmService vmService = await vmServiceConnectUri(
      'ws://localhost:${info.serverUri!.port}${info.serverUri!.path}ws',
    );

    // Spawns an isolate whose job is to send Http requests
    // This isolate will pause on Exceptions
    final ReceivePort receivePort = ReceivePort();
    final Isolate httpIsolate = await Isolate.spawn(
      httpIsolateEntryPoint,
      receivePort.sendPort,
      debugName: 'loopback',
    );
    SendPort? httpIsolatePort;

    // Setup ports and completers to communicate with the isolate
    final Map<String, Completer<bool>> requestCompleters = <String, Completer<bool>>{};
    receivePort.listen((dynamic data) async {
      if (data is SendPort) {
        httpIsolatePort = data;
      }
      if (data is String) {
        requestCompleters[data]!.complete(true);
      }
    });

    bool stoppedOnArgumentError = false;

    // Setup the VM to detect ArgumentError exceptions
    final vms.VM vm = await vmService.getVM();
    final vms.IsolateRef isolateRef = vm.isolates!.firstWhere(
        (vms.IsolateRef isolate) => isolate.name == httpIsolate.debugName);
    await vmService.setExceptionPauseMode(isolateRef.id!, 'All');
    await vmService.streamListen(vms.EventStreams.kDebug);
    vmService.onDebugEvent.listen((vms.Event event) async {
      if (event.kind == vms.EventKind.kPauseException) {
        if (event.exception?.classRef?.name == 'ArgumentError') {
          stoppedOnArgumentError = true;
        }
        await vmService.resume(isolateRef.id!);
      }
    });

    // Calls the isolate and wait for its response
    Future<void> sendRequest(String host, {bool? zoneAllowHttp}) async {
      stoppedOnArgumentError = false;
      requestCompleters[host] = Completer<bool>();
      httpIsolatePort!.send(zoneAllowHttp);
      httpIsolatePort!.send(host);
      await requestCompleters[host]!.future;
    }

    // Using an hostname should not reached an ArgumentError
    // if http requests are allowed at zone level
    await sendRequest(Platform.localHostname, zoneAllowHttp: true);
    expect(stoppedOnArgumentError, false);

    // Using an hostname should reached an ArgumentError
    // if http requests are not allowed at zone level
    await sendRequest(Platform.localHostname, zoneAllowHttp: false);
    expect(stoppedOnArgumentError, true);

    // Using an hostname should not reached an ArgumentError
    // if http requests are allowed at engine level
    await sendRequest(Platform.localHostname);
    expect(stoppedOnArgumentError, false);

    // Using an IP address should not reached an ArgumentError
    await sendRequest(await getLocalHostIP());
    expect(stoppedOnArgumentError, false);

    httpIsolate.kill();
    receivePort.close();
    await vmService.dispose();
  });
}

// Setup an Isolate that waits for : hostnames and allowHttp flag.
// For each hostname, spawns an HttpServer and sends a request to it
Future<void> httpIsolateEntryPoint(SendPort mainPort) async {
  final ReceivePort isolatePort = ReceivePort();
  mainPort.send(isolatePort.sendPort);
  bool? allowHttp;

  isolatePort.listen((dynamic data) async {
    if (data is bool?) {
      allowHttp = data;
    }
    if (data is String) {
      final String host = data;
      await bindServerAndTest(host, (HttpClient httpClient, Uri uri) async {
        await runZonedGuarded(
          () async {
            await httpClient.getUrl(uri);
            mainPort.send(host);
          },
          (_, __) => mainPort.send(host),
          zoneValues: <dynamic, dynamic>{#flutter.io.allow_http: allowHttp},
        );
      });
    }
  });
}
