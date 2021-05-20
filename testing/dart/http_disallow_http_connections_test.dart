// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9

// FlutterTesterOptions=--disallow-insecure-connections

import 'dart:async';
import 'dart:io';

import 'package:test/test.dart';

import 'test_util.dart';

Future<String> getLocalHostIP() async {
  final interfaces = await NetworkInterface.list(
      includeLoopback: false, type: InternetAddressType.IPv4);
  return interfaces.first.addresses.first.address;
}

Future<void> bindServerAndTest(String serverHost,
    Future<void> testCode(HttpClient client, Uri uri)) async {
  final httpClient = new HttpClient();
  final server = await HttpServer.bind(serverHost, 0);
  final uri = Uri(scheme: 'http', host: serverHost, port: server.port);
  try {
    await testCode(httpClient, uri);
  } finally {
    httpClient.close(force: true);
    await server.close();
  }
}

// Future<void> testWithLoopback() async {
//   await testBanHttp("127.0.0.1", (httpClient, uri) async {
//     await asyncTest(
//         () => httpClient.getUrl(Uri.parse('http://localhost:${uri.port}')));
//     await asyncTest(
//         () => httpClient.getUrl(Uri.parse('http://127.0.0.1:${uri.port}')));
//   });
// }

// Future<void> testWithIPv6() async {
//   if (await supportsIPV6()) {
//     await testBanHttp("::1", (httpClient, uri) async {
//       await asyncTest(() => httpClient.getUrl(uri));
//     });
//   }
// }

// Future<void> testWithHTTPS() async {
//   await testBanHttp(await getLocalHostIP(), (httpClient, uri) async {
//     asyncExpectThrows(
//         () => httpClient.getUrl(Uri(
//               scheme: 'https',
//               host: uri.host,
//               port: uri.port,
//             )),
//         (e) => e is SocketException || e is HandshakeException);
//   });
// }

/// Returns whether this computer supports binding to IPv6 addresses.
Future<bool> _supportsIPv6() async {
  try {
    var socket = await ServerSocket.bind(InternetAddress.loopbackIPv6, 0);
    await socket.close();
    return true;
  } on SocketException catch (_) {
    return false;
  }
}

main() {
  test('testWithHostname', () async {
    await bindServerAndTest(await getLocalHostIP(), (httpClient, httpUri) async {
      await asyncExpectThrows<UnsupportedError>(
          () async =>  httpClient.getUrl(httpUri));
      await asyncExpectThrows<UnsupportedError>(
          () async => runZoned(() => httpClient.getUrl(httpUri),
            zoneValues: {#flutter.io.allow_http: 'foo'}));
      await asyncExpectThrows<UnsupportedError>(
          () async => runZoned(() => httpClient.getUrl(httpUri),
            zoneValues: {#flutter.io.allow_http: false}));
      await runZoned(() => httpClient.getUrl(httpUri),
        zoneValues: {#flutter.io.allow_http: true});
    });
  });

  test('testWithLoopback', () async {
    await bindServerAndTest("127.0.0.1", (httpClient, uri) async {
      await httpClient.getUrl(Uri.parse('http://localhost:${uri.port}'));
      await httpClient.getUrl(Uri.parse('http://127.0.0.1:${uri.port}'));
    });
  });

  test('testWithIPV6', () async {
    if (await _supportsIPv6()) {
      await bindServerAndTest("::1", (httpClient, uri) async {
        await httpClient.getUrl(uri);
      });
    }    
  });
}
