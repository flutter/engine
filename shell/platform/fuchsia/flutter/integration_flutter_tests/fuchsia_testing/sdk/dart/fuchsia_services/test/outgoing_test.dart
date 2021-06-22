// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: implementation_imports
import 'dart:async';
import 'dart:convert' show utf8;
import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';
import 'package:fuchsia_services/src/outgoing.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart' as io;
import 'package:fuchsia_vfs/vfs.dart';
import 'package:test/test.dart';

void main() {
  late StreamController<bool> _streamController;
  late Stream<bool> _stream;

  setUp(() {
    _streamController = StreamController<bool>.broadcast();
    _stream = _streamController.stream;
  });

  tearDown(() {
    _streamController.close();
  });

  group('outgoing', () {
    test('connect to service calls correct service', () async {
      final outgoingImpl = Outgoing();
      final dirProxy = DirectoryProxy();
      outgoingImpl
        ..addPublicService(
          (_) {
            _streamController.add(true);
          },
          'foo',
        )
        ..serve(InterfaceRequest(dirProxy.ctrl.request().passChannel()));
      {
        final nodeProxy = NodeProxy();
        await dirProxy.open(0, 0, 'public/foo', nodeProxy.ctrl.request());
      }
      {
        final nodeProxy = NodeProxy();
        await dirProxy.open(0, 0, 'svc/foo', nodeProxy.ctrl.request());
      }
      _stream.listen(expectAsync1((response) {
        expect(response, true);
      }, count: 2));
    });

    test('diagnostics dir', () async {
      final outgoingImpl = Outgoing();
      final dirProxy = DirectoryProxy();
      outgoingImpl
          .diagnosticsDir()
          .addNode('foo', PseudoFile.readOnlyStr(() => 'test'));
      outgoingImpl
          .serve(InterfaceRequest(dirProxy.ctrl.request().passChannel()));
      final fileProxy = io.FileProxy();
      await dirProxy.open(
          io.openRightReadable,
          io.modeTypeFile,
          'diagnostics/foo',
          InterfaceRequest<io.Node>(fileProxy.ctrl.request().passChannel()));
      final response = await fileProxy.read(io.maxBuf);
      expect(utf8.decode(response.data), 'test');
    });
  });
}
