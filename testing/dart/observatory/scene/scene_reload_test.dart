// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:developer' as developer;
import 'dart:typed_data';
import 'dart:ui';

import 'package:litetest/litetest.dart';
import 'package:vm_service/vm_service.dart' as vms;
import 'package:vm_service/vm_service_io.dart';

void main() {
  test('ipscenes can be re-initialized', () async {
    vms.VmService? vmService;
    SceneShader? shader;
    try {
      final Float64List transform = Float64List(16);
      transform[0] = 1.0; // Scale X
      transform[5] = 1.0; // Scale Y
      transform[10] = 1.0; // Scale Z
      transform[12] = 2.0; // Translation X
      transform[13] = 6.0; // Translation Y
      transform[14] = 6.0; // Translation Z
      transform[15] = 1.0;
      final SceneNode sceneNode = await SceneNode.fromAsset(
        'flutter_logo.glb.ipscene',
      )..setTransform(transform);
      shader = sceneNode.sceneShader();
      _use(shader);

      final developer.ServiceProtocolInfo info = await developer.Service.getInfo();

      if (info.serverUri == null) {
        fail('This test must not be run with --disable-observatory.');
      }

      vmService = await vmServiceConnectUri(
        'ws://localhost:${info.serverUri!.port}${info.serverUri!.path}ws',
      );
      final vms.VM vm = await vmService.getVM();

      expect(vm.isolates!.isNotEmpty, true);
      for (final vms.IsolateRef isolateRef in vm.isolates!) {
        final vms.Response response = await vmService.callServiceExtension(
          'ext.ui.window.reinitializeScene',
          isolateId: isolateRef.id,
          args: <String, Object>{
            'assetKey': 'flutter_logo.glb.ipscene',
          },
        );
        expect(response.type == 'Success', true);
      }
    } finally {
      await vmService?.dispose();
      shader?.dispose();
    }
  });
}

void _use(Shader shader) {

}
