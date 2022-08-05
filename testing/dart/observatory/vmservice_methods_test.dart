// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:developer' as developer;

import 'package:litetest/litetest.dart';
import 'package:vm_service/vm_service.dart' as vms;
import 'package:vm_service/vm_service_io.dart';

void main() {
  test('Setting invalid directory returns an error', () async {
    vms.VmService? vmService;
    try {
      final developer.ServiceProtocolInfo info = await developer.Service.getInfo();
      if (info.serverUri == null) {
        fail('This test must not be run with --disable-observatory.');
      }

      vmService = await vmServiceConnectUri(
        'ws://localhost:${info.serverUri!.port}${info.serverUri!.path}ws',
      );
      final vms.Response response = await vmService.callMethod('_flutter.listViews');
      final List<Object?>? rawViews = response.json!['views'] as List<Object?>?;
      final String viewId = (rawViews![0]! as Map<String, Object?>?)!['id']! as String;

      final vms.Response setAssetDirectoryPath = await vmService.callMethod(
        '_flutter.setAssetBundlePath',
        args: <String, Object>{
          'viewId': viewId,
          'assetDirectory': ''
        },
      );
      expect(setAssetDirectoryPath.type == 'Failure', true);
    } finally {
      await vmService?.dispose();
    }
  });
}
