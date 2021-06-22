// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';
import 'package:fuchsia_vfs/vfs.dart';
import 'package:test/test.dart';
import 'package:zircon/zircon.dart';

void main() {
  group('pseudo dir: ', () {
    test('inode number', () {
      Vnode dir = PseudoDir();
      expect(dir.inodeNumber(), inoUnknown);
    });

    test('type', () {
      Vnode dir = PseudoDir();
      expect(dir.type(), direntTypeDirectory);
    });

    test('basic', () {
      PseudoDir dir = PseudoDir();
      var key1 = 'key1';
      var key2 = 'key2';

      var node1 = _TestVnode();
      expect(dir.addNode(key1, node1), ZX.OK);
      expect(dir.lookup(key1), node1);

      var node2 = _TestVnode();
      expect(dir.addNode(key2, node2), ZX.OK);
      expect(dir.lookup(key2), node2);

      // make sure key1 is still there
      expect(dir.lookup(key1), node1);
    });

    test('legal name', () {
      PseudoDir dir = PseudoDir();
      const maxObjectNameLength = 255;
      StringBuffer specialsNonPrintablesBuilder = StringBuffer();
      // null character is illegal, start at one
      for (var char = 1; char < maxObjectNameLength; char++) {
        if (char != 47 /* key seperator */) {
          specialsNonPrintablesBuilder.writeCharCode(char);
        }
      }
      var legalKeys = <String>[
        'k',
        'key',
        'longer_key',
        // dart linter prefers interpolation over cat
        'just_shy_of_max_key${'_' * 236}',
        '.prefix_is_independently_illegal',
        '..prefix_is_independently_illegal',
        'suffix_is_independetly_illegal.',
        'suffix_is_independetly_illegal..',
        'infix_is_._independetly_illegal',
        'infix_is_.._independetly_illegal',
        '...',
        '....',
        'space is legal',
        'which\tmakes\tme\tuncomfortable',
        'very\nuncomfortable',
        'numbers_0123456789',
        specialsNonPrintablesBuilder.toString(),
      ];
      for (var key in legalKeys) {
        var node = _TestVnode();
        expect(dir.addNode(key, node), ZX.OK);
        expect(dir.lookup(key), node);
      }
    });

    test('illegal name', () {
      PseudoDir dir = PseudoDir();
      var illegalKeys = <String>[
        '',
        'illegal_length_key${'_' * 238}',
        '.',
        '..',
        '\u{00}',
        'null_\u{00}_character',
        '/',
        'key_/_seperator',
      ];
      var node = _TestVnode();
      for (var key in illegalKeys) {
        expect(dir.addNode(key, node), ZX.ERR_INVALID_ARGS);
      }
    });

    test('duplicate key', () {
      PseudoDir dir = PseudoDir();
      var key = 'key';
      var node = _TestVnode();
      var dupNode = _TestVnode();
      expect(dir.addNode(key, node), ZX.OK);
      expect(dir.addNode(key, dupNode), ZX.ERR_ALREADY_EXISTS);

      // check that key was not replaced
      expect(dir.lookup(key), node);
    });

    test('remove node', () {
      PseudoDir dir = PseudoDir();
      var key = 'key';
      var node = _TestVnode();
      expect(dir.addNode(key, node), ZX.OK);
      expect(dir.lookup(key), node);

      expect(dir.removeNode(key), ZX.OK);
      expect(dir.lookup(key), null);

      // add again and check
      expect(dir.addNode(key, node), ZX.OK);
      expect(dir.lookup(key), node);
    });

    test('remove when multiple keys', () {
      PseudoDir dir = PseudoDir();
      var key1 = 'key1';
      var key2 = 'key2';
      var node1 = _TestVnode();
      var node2 = _TestVnode();
      expect(dir.addNode(key1, node1), ZX.OK);
      expect(dir.addNode(key2, node2), ZX.OK);
      expect(dir.lookup(key1), node1);
      expect(dir.lookup(key2), node2);

      expect(dir.removeNode(key1), ZX.OK);
      expect(dir.lookup(key1), null);

      // check that key2 is still there
      expect(dir.lookup(key2), node2);

      // add again and check
      expect(dir.addNode(key1, node1), ZX.OK);
      expect(dir.lookup(key1), node1);
      expect(dir.lookup(key2), node2);
    });

    test('key order is maintained', () {
      PseudoDir dir = PseudoDir();
      var key1 = 'key1';
      var key2 = 'key2';
      var key3 = 'key3';
      var node1 = _TestVnode();
      var node2 = _TestVnode();
      var node3 = _TestVnode();
      expect(dir.addNode(key1, node1), ZX.OK);
      expect(dir.addNode(key2, node2), ZX.OK);
      expect(dir.addNode(key3, node3), ZX.OK);

      expect(dir.listNodeNames(), [key1, key2, key3]);

      // order maintained after removing node
      expect(dir.removeNode(key1), ZX.OK);
      expect(dir.listNodeNames(), [key2, key3]);

      // add again and check
      expect(dir.addNode(key1, node1), ZX.OK);
      expect(dir.listNodeNames(), [key2, key3, key1]);
    });

    test('remove and isEmpty', () {
      PseudoDir dir = PseudoDir();
      var key1 = 'key1';
      var key2 = 'key2';
      var key3 = 'key3';
      var node1 = _TestVnode();
      var node2 = _TestVnode();
      var node3 = _TestVnode();
      expect(dir.isEmpty(), true);
      expect(dir.addNode(key1, node1), ZX.OK);
      expect(dir.addNode(key2, node2), ZX.OK);
      expect(dir.addNode(key3, node3), ZX.OK);
      expect(dir.isEmpty(), false);

      expect(dir.removeNode(key1), ZX.OK);
      expect(dir.isEmpty(), false);
      dir.removeAllNodes();
      expect(dir.isEmpty(), true);
      expect(dir.listNodeNames(), []);
      // make sure that keys are really gone
      expect(dir.lookup(key2), null);
      expect(dir.lookup(key3), null);

      // add again and check
      expect(dir.addNode(key1, node1), ZX.OK);
      expect(dir.isEmpty(), false);
      expect(dir.lookup(key1), node1);
      expect(dir.listNodeNames(), [key1]);
    });
  });

  group('pseudo dir server: ', () {
    group('open fails: ', () {
      test('invalid flags', () async {
        PseudoDir dir = PseudoDir();
        var invalidFlags = [
          openFlagAppend,
          openFlagCreate,
          openFlagCreateIfAbsent,
          openFlagNoRemote,
          openFlagTruncate,
          openRightAdmin,
        ];

        var i = 0;
        for (var flag in invalidFlags) {
          DirectoryProxy proxy = DirectoryProxy();
          var status = dir.connect(flag | openFlagDescribe, 0,
              InterfaceRequest(proxy.ctrl.request().passChannel()));
          expect(status, isNot(ZX.OK), reason: 'flagIndex: $i');
          i++;
          await proxy.onOpen.first.then((response) {
            expect(response.s, status);
            expect(response.info, isNull);
          }).catchError((err) async {
            fail(err.toString());
          });
        }
      });

      test('invalid mode', () async {
        PseudoDir dir = PseudoDir();
        var invalidModes = [
          modeTypeBlockDevice,
          modeTypeFile,
          modeTypeService,
          modeTypeService,
          modeTypeSocket
        ];

        var i = 0;
        for (var mode in invalidModes) {
          DirectoryProxy proxy = DirectoryProxy();
          var status = dir.connect(openFlagDescribe, mode,
              InterfaceRequest(proxy.ctrl.request().passChannel()));
          expect(status, ZX.ERR_INVALID_ARGS, reason: 'modeIndex: $i');
          i++;
          await proxy.onOpen.first.then((response) {
            expect(response.s, status);
            expect(response.info, isNull);
          }).catchError((err) async {
            fail(err.toString());
          });
        }
      });
    });

    DirectoryProxy _getProxyForDir(PseudoDir dir,
        [int flags = openRightReadable | openRightWritable]) {
      DirectoryProxy proxy = DirectoryProxy();
      var status = dir.connect(
          flags, 0, InterfaceRequest(proxy.ctrl.request().passChannel()));
      expect(status, ZX.OK);
      return proxy;
    }

    test('open passes', () async {
      PseudoDir dir = PseudoDir();
      DirectoryProxy proxy =
          _getProxyForDir(dir, openRightReadable | openFlagDescribe);

      await proxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('open passes with valid mode', () async {
      PseudoDir dir = PseudoDir();
      var validModes = [
        modeProtectionMask,
        modeTypeDirectory,
      ];

      var i = 0;
      for (var mode in validModes) {
        DirectoryProxy proxy = DirectoryProxy();
        var status = dir.connect(openFlagDescribe, mode,
            InterfaceRequest(proxy.ctrl.request().passChannel()));
        expect(status, ZX.OK, reason: 'modeIndex: $i');
        i++;
        await proxy.onOpen.first.then((response) {
          expect(response.s, ZX.OK);
          expect(response.info, isNotNull);
        }).catchError((err) async {
          fail(err.toString());
        });
      }
    });

    test('open passes with valid flags', () async {
      PseudoDir dir = PseudoDir();
      var validFlags = [
        openRightReadable,
        openRightWritable,
        openRightReadable | openFlagDirectory,
        openFlagNodeReference
      ];

      for (var flag in validFlags) {
        DirectoryProxy proxy = _getProxyForDir(dir, flag | openFlagDescribe);
        await proxy.onOpen.first.then((response) {
          expect(response.s, ZX.OK);
          expect(response.info, isNotNull);
        }).catchError((err) async {
          fail(err.toString());
        });
      }
    });

    test('getattr', () async {
      PseudoDir dir = PseudoDir();
      DirectoryProxy proxy = _getProxyForDir(dir);

      var attr = await proxy.getAttr();

      expect(attr.attributes.linkCount, 1);
      expect(attr.attributes.mode, modeProtectionMask | modeTypeDirectory);
    });

    _Dirent _createDirentForDot() {
      return _Dirent(inoUnknown, 1, direntTypeDirectory, '.');
    }

    _Dirent _createDirent(Vnode vnode, String name) {
      return _Dirent(vnode.inodeNumber(), name.length, vnode.type(), name);
    }

    int _expectedDirentSize(List<_Dirent> dirents) {
      var sum = 0;
      for (var d in dirents) {
        sum += d.direntSizeInBytes!;
      }
      return sum;
    }

    void _validateExpectedDirents(
        List<_Dirent> dirents, Directory$ReadDirents$Response response) {
      expect(response.s, ZX.OK);
      expect(response.dirents.length, _expectedDirentSize(dirents));
      var offset = 0;
      for (var dirent in dirents) {
        var data = ByteData.view(
            response.dirents.buffer, response.dirents.offsetInBytes + offset);
        var actualDirent = _Dirent.fromData(data);
        expect(actualDirent, dirent);
        offset += actualDirent.direntSizeInBytes!;
      }
    }

    group('read dir:', () {
      test('simple call should work', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        var file2 = PseudoFile.readOnlyStr(() => 'file2');
        var file3 = PseudoFile.readOnlyStr(() => 'file3');
        dir
          ..addNode('file1', file1)
          ..addNode('subDir', subDir)
          ..addNode('file3', file3);
        subDir.addNode('file2', file2);

        DirectoryProxy proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
          _createDirent(file3, 'file3'),
        ];
        var response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);

        // test that next read call returns length zero buffer
        response = await proxy.readDirents(1024);
        expect(response.s, ZX.OK);
        expect(response.dirents.length, 0);

        // also test sub folder and make sure it was not affected by parent dir.
        proxy = _getProxyForDir(subDir);
        expectedDirents = [
          _createDirentForDot(),
          _createDirent(file2, 'file2'),
        ];
        response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('serve function works', () async {
        PseudoDir dir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');

        dir.addNode('file1', file1);

        DirectoryProxy proxy = DirectoryProxy();
        var status =
            dir.serve(InterfaceRequest(proxy.ctrl.request().passChannel()));
        expect(status, ZX.OK);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
        ];
        var response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('passed buffer size is exact', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        var file3 = PseudoFile.readOnlyStr(() => 'file3');
        dir
          ..addNode('file1', file1)
          ..addNode('subDir', subDir)
          ..addNode('file3', file3);
        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
          _createDirent(file3, 'file3'),
        ];
        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents));
        _validateExpectedDirents(expectedDirents, response);

        // test that next read call returns length zero buffer
        response = await proxy.readDirents(1024);
        expect(response.s, ZX.OK);
        expect(response.dirents.length, 0);
      });

      test('passed buffer size is exact - 1', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        var file3 = PseudoFile.readOnlyStr(() => 'file3');
        dir
          ..addNode('file1', file1)
          ..addNode('subDir', subDir)
          ..addNode('file3', file3);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
          _createDirent(file3, 'file3'),
        ];
        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents) - 1);
        var lastDirent = expectedDirents.removeLast();
        _validateExpectedDirents(expectedDirents, response);

        // test that next read call returns last dirent
        response = await proxy.readDirents(1024);
        _validateExpectedDirents([lastDirent], response);

        // test that next read call returns length zero buffer
        response = await proxy.readDirents(1024);
        expect(response.s, ZX.OK);
        expect(response.dirents.length, 0);
      });

      test('buffer too small', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var size = _expectedDirentSize([_createDirentForDot()]) - 1;
        for (int i = 0; i < size; i++) {
          var response = await proxy.readDirents(i);
          expect(response.s, ZX.ERR_BUFFER_TOO_SMALL);
          expect(response.dirents.length, 0);
        }
      });

      test(
          'buffer too small after first dot read and subsequent reads with bigger buffer returns correct dirents',
          () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);
        var size = _expectedDirentSize([_createDirentForDot()]);
        var response = await proxy.readDirents(size);

        // make sure that '.' was read
        _validateExpectedDirents([_createDirentForDot()], response);

        // this should return error
        response = await proxy.readDirents(size);
        expect(response.s, ZX.ERR_BUFFER_TOO_SMALL);
        expect(response.dirents.length, 0);

        var expectedDirents = [
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
        ];
        response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents));
        _validateExpectedDirents(expectedDirents, response);
      });

      test('multiple reads with small buffer', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);
        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
        ];
        for (var dirent in expectedDirents) {
          var dirents = [dirent];
          var response = await proxy.readDirents(_expectedDirentSize(dirents));
          _validateExpectedDirents(dirents, response);
        }

        // test that next read call returns length zero buffer
        var response = await proxy.readDirents(1024);
        expect(response.s, ZX.OK);
        expect(response.dirents.length, 0);
      });

      test('read two dirents then one', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents));
        _validateExpectedDirents(expectedDirents, response);

        expectedDirents = [
          _createDirent(subDir, 'subDir'),
        ];

        response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('buffer size more than first less than 2 dirents', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents) + 10);
        _validateExpectedDirents(expectedDirents, response);

        // now test that we are able to get rest
        expectedDirents = [
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
        ];

        response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('rewind works', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents) + 10);
        _validateExpectedDirents(expectedDirents, response);

        var rewindResponse = await proxy.rewind();
        expect(rewindResponse, ZX.OK);

        response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents) + 10);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('rewind works after we reach directory end', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents) + 10);
        _validateExpectedDirents(expectedDirents, response);

        var rewindResponse = await proxy.rewind();
        expect(rewindResponse, ZX.OK);

        response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents) + 10);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('readdir works when node removed', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents));
        _validateExpectedDirents(expectedDirents, response);

        // remove first node
        dir.removeNode('file1');
        expectedDirents = [_createDirent(subDir, 'subDir')];
        response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('readdir works when already last node is removed', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1')
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents));
        _validateExpectedDirents(expectedDirents, response);

        // remove first node
        dir.removeNode('file1');
        expectedDirents = [_createDirent(subDir, 'subDir')];
        response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('readdir works when node is added', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir')
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents));
        _validateExpectedDirents(expectedDirents, response);

        dir.addNode('file2', file1);
        expectedDirents = [_createDirent(file1, 'file2')];
        response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });

      test('readdir works when node is added and only first node was read',
          () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);

        var expectedDirents = [
          _createDirentForDot(),
        ];

        var response =
            await proxy.readDirents(_expectedDirentSize(expectedDirents));
        _validateExpectedDirents(expectedDirents, response);

        dir.addNode('file2', file1);
        expectedDirents = [
          _createDirent(file1, 'file1'),
          _createDirent(subDir, 'subDir'),
          _createDirent(file1, 'file2')
        ];
        response = await proxy.readDirents(1024);
        _validateExpectedDirents(expectedDirents, response);
      });
    });

    group('open/close file/dir in dir:', () {
      Future<void> _openFileAndAssert(DirectoryProxy proxy, String filePath,
          int bufferLen, String expectedContent) async {
        FileProxy fileProxy = FileProxy();
        await proxy.open(openRightReadable, 0, filePath,
            InterfaceRequest(fileProxy.ctrl.request().passChannel()));

        var readResonse = await fileProxy.read(bufferLen);
        expect(readResonse.s, ZX.OK);
        expect(String.fromCharCodes(readResonse.data), expectedContent);
      }

      PseudoDir _setUpDir() {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        var file2 = PseudoFile.readOnlyStr(() => 'file2');
        var file3 = PseudoFile.readOnlyStr(() => 'file3');
        var file4 = PseudoFile.readOnlyStr(() => 'file4');
        dir
          ..addNode('file1', file1)
          ..addNode('subDir', subDir)
          ..addNode('file3', file3);
        subDir..addNode('file2', file2)..addNode('file4', file4);
        return dir;
      }

      test('open self', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);
        var paths = ['.', './', './/', './//', './/.//./'];
        for (var path in paths) {
          DirectoryProxy newProxy = DirectoryProxy();
          await proxy.open(openRightReadable, 0, path,
              InterfaceRequest(newProxy.ctrl.request().passChannel()));

          // open file 1 in proxy and check contents to make sure correct dir was opened.
          await _openFileAndAssert(newProxy, 'file1', 100, 'file1');
        }
      });

      test('open file', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);

        // open file 1 check contents.
        var paths = ['file1', './file1', './/file1', './//file1'];
        for (var path in paths) {
          await _openFileAndAssert(proxy, path, 100, 'file1');
        }
      });

      test('open fails for illegal path', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);
        var paths = <String>[
          '',
          'too_long_path${'_' * 242}',
          'subDir/too_long_path${'_' * 242}',
          '..',
          'subDir/..',
          'invalid_\u{00}_name',
          'subDir/invalid_\u{00}_name',
          'invalid_\u{00}_name/legal_name',
        ];
        for (var path in paths) {
          DirectoryProxy newProxy = DirectoryProxy();
          await proxy.open(openRightReadable | openFlagDescribe, 0, path,
              InterfaceRequest(newProxy.ctrl.request().passChannel()));

          await newProxy.onOpen.first.then((response) {
            expect(response.s, isNot(ZX.OK));
            expect(response.info, isNull);
          }).catchError((err) async {
            fail(err.toString());
          });
        }
      });

      test('open file fails for path ending with "/"', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);

        FileProxy fileProxy = FileProxy();
        await proxy.open(openRightReadable | openFlagDescribe, 0, 'file1/',
            InterfaceRequest(fileProxy.ctrl.request().passChannel()));

        await fileProxy.onOpen.first.then((response) {
          expect(response.s, ZX.ERR_NOT_DIR);
          expect(response.info, isNull);
        }).catchError((err) async {
          fail(err.toString());
        });
      });

      test('open file fails for invalid key', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);

        FileProxy fileProxy = FileProxy();
        await proxy.open(openRightReadable, 0, 'invalid',
            InterfaceRequest(fileProxy.ctrl.request().passChannel()));

        // channel should be closed
        fileProxy.ctrl.whenClosed.asStream().listen(expectAsync1((_) {}));
      });

      test('open fails for trying to open file within a file', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);

        FileProxy fileProxy = FileProxy();
        await proxy.open(openRightReadable | openFlagDescribe, 0, 'file1/file2',
            InterfaceRequest(fileProxy.ctrl.request().passChannel()));

        await fileProxy.onOpen.first.then((response) {
          expect(response.s, ZX.ERR_NOT_DIR);
          expect(response.info, isNull);
        }).catchError((err) async {
          fail(err.toString());
        });
      });

      test('close works', () async {
        PseudoDir dir = PseudoDir();
        PseudoDir subDir = PseudoDir();
        var file1 = PseudoFile.readOnlyStr(() => 'file1');
        dir..addNode('file1', file1)..addNode('subDir', subDir);

        var proxy = _getProxyForDir(dir);
        DirectoryProxy subDirProxy = DirectoryProxy();
        await proxy.open(openRightReadable, 0, 'subDir',
            InterfaceRequest(subDirProxy.ctrl.request().passChannel()));

        FileProxy fileProxy = FileProxy();
        await proxy.open(openRightReadable, 0, 'file1',
            InterfaceRequest(fileProxy.ctrl.request().passChannel()));
        dir.close();
        proxy.ctrl.whenClosed.asStream().listen(expectAsync1((_) {}));
        subDirProxy.ctrl.whenClosed.asStream().listen(expectAsync1((_) {}));
        fileProxy.ctrl.whenClosed.asStream().listen(expectAsync1((_) {}));
      });

      test('open sub dir', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);

        DirectoryProxy dirProxy = DirectoryProxy();
        await proxy.open(openRightReadable, 0, 'subDir',
            InterfaceRequest(dirProxy.ctrl.request().passChannel()));

        // open file 2 check contents to make sure correct dir was opened.
        await _openFileAndAssert(dirProxy, 'file2', 100, 'file2');
      });

      test('directory rights are hierarchical (open dir)', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir, openRightReadable);

        var newProxy = DirectoryProxy();
        await proxy.open(openRightWritable | openFlagDescribe, 0, 'subDir',
            InterfaceRequest(newProxy.ctrl.request().passChannel()));

        await newProxy.onOpen.first.then((response) {
          expect(response.s, ZX.ERR_ACCESS_DENIED);
          expect(response.info, isNull);
        }).catchError((err) async {
          fail(err.toString());
        });
      });

      test('directory rights are hierarchical (open file)', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir, openRightWritable);

        var newProxy = DirectoryProxy();
        await proxy.open(openRightWritable | openFlagDescribe, 0, 'subDir',
            InterfaceRequest(newProxy.ctrl.request().passChannel()));

        await newProxy.onOpen.first.then((response) {
          expect(response.s, ZX.OK);
          expect(response.info, isNotNull);
        }).catchError((err) async {
          fail(err.toString());
        });

        FileProxy fileProxy = FileProxy();
        await newProxy.open(openRightReadable, 0, 'file2',
            InterfaceRequest(fileProxy.ctrl.request().passChannel()));

        // channel should be closed
        fileProxy.ctrl.whenClosed.asStream().listen(expectAsync1((_) {}));
      });

      test('open sub dir with "/" at end', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);

        DirectoryProxy dirProxy = DirectoryProxy();
        await proxy.open(openRightReadable, 0, 'subDir/',
            InterfaceRequest(dirProxy.ctrl.request().passChannel()));

        // open file 2 check contents to make sure correct dir was opened.
        await _openFileAndAssert(dirProxy, 'file2', 100, 'file2');
      });

      test('directly open file in sub dir', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir);

        // open file 2 in subDir.
        await _openFileAndAssert(proxy, 'subDir/file2', 100, 'file2');
      });

      test('readdir fails for NodeReference', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir, openFlagNodeReference);

        var response = await proxy.readDirents(1024);
        expect(response.s, ZX.ERR_BAD_HANDLE);
      });

      test('not allowed to open a file for NodeReference', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir, openFlagNodeReference);

        // open file 2 in subDir.
        FileProxy fileProxy = FileProxy();
        await proxy.open(openRightReadable, 0, 'subDir/file2',
            InterfaceRequest(fileProxy.ctrl.request().passChannel()));

        // channel should be closed
        fileProxy.ctrl.whenClosed.asStream().listen(expectAsync1((_) {}));
      });

      test('clone with same rights', () async {
        PseudoDir dir = _setUpDir();

        var proxy = _getProxyForDir(dir, openRightReadable | openRightWritable);
        DirectoryProxy cloneProxy = DirectoryProxy();
        await proxy.clone(cloneFlagSameRights | openFlagDescribe,
            InterfaceRequest(cloneProxy.ctrl.request().passChannel()));

        var subDirProxy = DirectoryProxy();
        await cloneProxy.open(
            openRightReadable | openRightWritable | openFlagDescribe,
            0,
            'subDir',
            InterfaceRequest(subDirProxy.ctrl.request().passChannel()));

        await subDirProxy.onOpen.first.then((response) {
          expect(response.s, ZX.OK);
          expect(response.info, isNotNull);
        }).catchError((err) async {
          fail(err.toString());
        });

        // open file 2 check contents to make sure correct dir was opened.
        await _openFileAndAssert(subDirProxy, 'file2', 100, 'file2');
      });
    });

    test('test clone', () async {
      PseudoDir dir = PseudoDir();

      var proxy = _getProxyForDir(dir, openRightReadable);

      DirectoryProxy newProxy = DirectoryProxy();
      await proxy.clone(openRightReadable | openFlagDescribe,
          InterfaceRequest(newProxy.ctrl.request().passChannel()));

      await newProxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('clone should fail if requested rights exceed source rights',
        () async {
      PseudoDir dir = PseudoDir();
      var proxy = _getProxyForDir(dir, openRightReadable);

      var clonedProxy = DirectoryProxy();
      await proxy.clone(openRightWritable | openFlagDescribe,
          InterfaceRequest(clonedProxy.ctrl.request().passChannel()));

      await clonedProxy.onOpen.first.then((response) {
        expect(response.s, ZX.ERR_ACCESS_DENIED);
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('test clone fails for invalid flags', () async {
      PseudoDir dir = PseudoDir();

      var proxy = _getProxyForDir(dir, openRightReadable);

      DirectoryProxy newProxy = DirectoryProxy();
      await proxy.clone(openFlagTruncate | openFlagDescribe,
          InterfaceRequest(newProxy.ctrl.request().passChannel()));

      await newProxy.onOpen.first.then((response) {
        expect(response.s, isNot(ZX.OK));
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('test clone disallows both cloneFlagSameRights and specific rights',
        () async {
      PseudoDir dir = PseudoDir();
      var proxy = _getProxyForDir(dir, openRightReadable);

      var clonedProxy = DirectoryProxy();
      await proxy.clone(
          openRightReadable | cloneFlagSameRights | openFlagDescribe,
          InterfaceRequest(clonedProxy.ctrl.request().passChannel()));

      await clonedProxy.onOpen.first.then((response) {
        expect(response.s, ZX.ERR_INVALID_ARGS);
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });
  });
}

class _Dirent {
  static const int _fixedSize = 10;
  int? ino;
  int? size;
  int? type;
  String? str;

  int? direntSizeInBytes;
  _Dirent(this.ino, this.size, this.type, this.str) {
    direntSizeInBytes = _fixedSize + size!;
  }

  _Dirent.fromData(ByteData data) {
    ino = data.getUint64(0, Endian.little);
    size = data.getUint8(8);
    type = data.getUint8(9);
    var offset = _fixedSize;
    List<int> charBytes = [];
    direntSizeInBytes = offset + size!;
    expect(data.lengthInBytes, greaterThanOrEqualTo(direntSizeInBytes));
    for (int i = 0; i < size!; i++) {
      charBytes.add(data.getUint8(offset++));
    }
    str = utf8.decode(charBytes);
  }

  @override
  int get hashCode =>
      ino.hashCode + size.hashCode + type.hashCode + str.hashCode;

  @override
  bool operator ==(Object o) {
    return o is _Dirent &&
        o.ino == ino &&
        o.size == size &&
        o.type == type &&
        o.str == str;
  }

  @override
  String toString() {
    return '[ino: $ino, size: $size, type: $type, str: $str]';
  }
}

class _TestVnode extends Vnode {
  final String _val;
  _TestVnode([this._val = '']);

  @override
  int connect(int flags, int mode, InterfaceRequest<Node> request,
      [int parentFlags =
          openRightReadable | openRightWritable | openRightAdmin]) {
    throw UnimplementedError();
  }

  @override
  int inodeNumber() {
    return inoUnknown;
  }

  @override
  int type() {
    return direntTypeUnknown;
  }

  String value() => _val;

  @override
  void close() {}
}
