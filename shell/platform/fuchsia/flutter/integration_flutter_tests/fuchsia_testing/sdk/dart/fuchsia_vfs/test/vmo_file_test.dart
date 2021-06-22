// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';

import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';
import 'package:fuchsia_vfs/vfs.dart';
import 'package:test/test.dart';
import 'package:zircon/zircon.dart';

void main() {
  InterfaceRequest<Node> _getNodeInterfaceRequest(FileProxy proxy) {
    return InterfaceRequest<Node>(proxy.ctrl.request().passChannel());
  }

  _ReadOnlyFile _createVmoFile(String str, openRights,
      [VmoSharingMode mode = VmoSharingMode.shareDuplicate]) {
    final SizedVmo sizedVmo =
        SizedVmo.fromUint8List(Uint8List.fromList(str.codeUnits));

    _ReadOnlyFile file = _ReadOnlyFile()
      ..vmoFile = VmoFile.readOnly(Vmo(sizedVmo.handle), mode)
      ..proxy = FileProxy();
    expect(
        file.vmoFile
            .connect(openRights, 0, _getNodeInterfaceRequest(file.proxy)),
        ZX.OK);
    return file;
  }

  Future<void> _assertRead(FileProxy proxy, int bufSize, String expectedStr,
      {expectedStatus = ZX.OK}) async {
    var readResponse = await proxy.read(bufSize);
    expect(readResponse.s, expectedStatus);
    expect(String.fromCharCodes(readResponse.data), expectedStr);
  }

  Future<void> _assertDescribeFile(FileProxy proxy) async {
    var response = await proxy.describe();
    expect(response.file, isNotNull);
  }

  Future<void> _assertDescribeVmo(FileProxy proxy, String expectedStr) async {
    var response = await proxy.describe();
    Vmofile? vmoFile = response.vmofile;
    if (vmoFile != null) {
      expect(vmoFile.vmo.isValid, isTrue);
      final Uint8List value = vmoFile.vmo.map();
      expect(String.fromCharCodes(value.sublist(0, expectedStr.length)),
          expectedStr);
    }
  }

  group('vmo file:', () {
    test('onOpen event on success', () async {
      var file =
          _createVmoFile('test_str', openRightReadable | openFlagDescribe);

      await file.proxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('onOpen with describe flag', () async {
      var file =
          _createVmoFile('test_str', openRightReadable | openFlagDescribe);

      await file.proxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        NodeInfo? nodeInfo = response.info;
        if (nodeInfo != null) {
          expect(nodeInfo.vmofile, isNotNull);
        }
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('read file', () async {
      var str = 'test_str';
      var file = _createVmoFile(str, openRightReadable);
      await _assertRead(file.proxy, str.length, str);
    });

    test('describe duplicate', () async {
      var str = 'test_str';
      var file = _createVmoFile(str, openRightReadable);
      await _assertDescribeVmo(file.proxy, str);
    });

    test('describe no sharing', () async {
      var str = 'test_str';
      var file =
          _createVmoFile(str, openRightReadable, VmoSharingMode.noSharing);
      await _assertDescribeFile(file.proxy);
    });
  });
}

class _ReadOnlyFile {
  late VmoFile vmoFile;
  late FileProxy proxy;
}
