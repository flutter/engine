// Copyright 2018 The Fuchsia Authors. All rights reserved.
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
  const _newStr = 'new_str';
  final _newStrList = Uint8List.fromList(_newStr.codeUnits);

  InterfaceRequest<Node> _getNodeInterfaceRequest(FileProxy proxy) {
    return InterfaceRequest<Node>(proxy.ctrl.request().passChannel());
  }

  _ReadOnlyFile _createReadOnlyFile(String str, int openFlags,
      [int expectedStatus = ZX.OK]) {
    _ReadOnlyFile file = _ReadOnlyFile()
      ..pseudoFile = PseudoFile.readOnlyStr(() {
        return str;
      })
      ..proxy = FileProxy();
    expect(
        file.pseudoFile
            .connect(openFlags, 0, _getNodeInterfaceRequest(file.proxy)),
        expectedStatus);
    return file;
  }

  Future<void> _assertFinalBuffer(FileProxy proxy, _ReadWriteFile file,
      String oldStr, String newStr) async {
    // our buffer should contain old string
    expect(file.buffer, oldStr);

    var closeResponse = await proxy.close();
    expect(closeResponse, ZX.OK);

    // our buffer should contain new string
    expect(file.buffer, newStr);
  }

  Future<void> _assertWrite(FileProxy proxy, Uint8List content,
      {int expectedStatus = ZX.OK, int? expectedSize}) async {
    expectedSize ??= content.length;
    var writeResponse = await proxy.write(content);
    expect(writeResponse.s, expectedStatus);
    expect(writeResponse.actual, expectedSize);
  }

  Future<void> _assertRead(FileProxy proxy, int bufSize, String expectedStr,
      {expectedStatus = ZX.OK}) async {
    var readResponse = await proxy.read(bufSize);
    expect(readResponse.s, expectedStatus);
    expect(String.fromCharCodes(readResponse.data), expectedStr);
  }

  Future<void> _assertReadAt(
      FileProxy proxy, int bufSize, int offset, String expectedStr,
      {expectedStatus = ZX.OK}) async {
    var readAtResponse = await proxy.readAt(bufSize, offset);
    expect(readAtResponse.s, expectedStatus);
    expect(String.fromCharCodes(readAtResponse.data), expectedStr);
  }

  Future<void> _assertWriteAt(
      FileProxy proxy, Uint8List content, int offset, int expectedWrittenLen,
      {expectedStatus = ZX.OK}) async {
    var writeAtResponse = await proxy.writeAt(content, offset);
    expect(writeAtResponse.s, expectedStatus);
    expect(writeAtResponse.actual, expectedWrittenLen);
  }

  group('pseudo file creation validation: ', () {
    PseudoFile _createReadWriteFileStub() {
      return PseudoFile.readWriteStr(1, () {
        return '';
      }, (String str) {
        return ZX.OK;
      });
    }

    var _notAllowedFlags = [
      openFlagCreate,
      openFlagCreateIfAbsent,
      openFlagNoRemote,
      openRightAdmin
    ];

    test('onOpen event on flag validation error', () async {
      var file = _createReadOnlyFile(
          '', openRightWritable | openFlagDescribe, ZX.ERR_NOT_SUPPORTED);

      await file.proxy.onOpen.first.then((response) {
        expect(response.s, ZX.ERR_NOT_SUPPORTED);
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('read only file', () async {
      var file =
          _createReadOnlyFile('', openRightWritable, ZX.ERR_NOT_SUPPORTED);

      var proxy = FileProxy();
      expect(
          file.pseudoFile
              .connect(openFlagTruncate, 0, _getNodeInterfaceRequest(proxy)),
          ZX.ERR_NOT_SUPPORTED);

      proxy = FileProxy();
      expect(
          file.pseudoFile
              .connect(openFlagDirectory, 0, _getNodeInterfaceRequest(proxy)),
          ZX.ERR_NOT_DIR);

      proxy = FileProxy();
      expect(
          file.pseudoFile
              .connect(openFlagAppend, 0, _getNodeInterfaceRequest(proxy)),
          ZX.ERR_INVALID_ARGS);

      for (var flag in _notAllowedFlags) {
        proxy = FileProxy();
        expect(
            file.pseudoFile.connect(flag, 0, _getNodeInterfaceRequest(proxy)),
            ZX.ERR_NOT_SUPPORTED,
            reason: 'for flag: $flag');
      }
    });

    test('read write file', () async {
      var file = _createReadWriteFileStub();

      var proxy = FileProxy();
      expect(
          file.connect(openFlagDirectory, 0, _getNodeInterfaceRequest(proxy)),
          ZX.ERR_NOT_DIR);

      proxy = FileProxy();
      expect(file.connect(openFlagAppend, 0, _getNodeInterfaceRequest(proxy)),
          ZX.ERR_INVALID_ARGS);

      for (var flag in _notAllowedFlags) {
        proxy = FileProxy();
        expect(file.connect(flag, 0, _getNodeInterfaceRequest(proxy)),
            ZX.ERR_NOT_SUPPORTED,
            reason: 'for flag: $flag');
      }
    });

    test('connect file with mode', () async {
      var file = _createReadWriteFileStub();

      var proxy = FileProxy();
      expect(
          file.connect(openRightReadable | openFlagDescribe, ~modeTypeFile,
              _getNodeInterfaceRequest(proxy)),
          ZX.ERR_INVALID_ARGS);

      await proxy.onOpen.first.then((response) {
        expect(response.s, ZX.ERR_INVALID_ARGS);
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });

      proxy = FileProxy();
      expect(
          file.connect(openRightReadable | openFlagDescribe, modeTypeFile,
              _getNodeInterfaceRequest(proxy)),
          ZX.OK);
      await proxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('open fails', () async {
      var file = _createReadWriteFileStub();

      var paths = ['', '/', '.', './', './/', './//'];
      for (var path in paths) {
        var proxy = FileProxy();
        file.open(openRightReadable | openFlagDescribe, 0, path,
            _getNodeInterfaceRequest(proxy), openRightReadable);

        await proxy.onOpen.first.then((response) {
          expect(response.s, ZX.ERR_NOT_DIR);
          expect(response.info, isNull);
        }).catchError((err) async {
          fail(err.toString());
        });
      }
    });
  });

  group('pseudo file:', () {
    _ReadWriteFile _createReadWriteFile(String initialStr,
        {int? capacity,
        createProxy = true,
        flags = openRightReadable | openRightWritable}) {
      int c = initialStr.length;
      if (capacity != null) {
        assert(capacity >= initialStr.length);
        c = capacity;
      }
      _ReadWriteFile file = _ReadWriteFile();
      file
        ..buffer = initialStr
        ..pseudoFile = PseudoFile.readWriteStr(c, () {
          return file.buffer;
        }, (String str) {
          file.buffer = str;
          return ZX.OK;
        });
      if (createProxy) {
        file.proxy = FileProxy();
        expect(
            file.pseudoFile
                .connect(flags, 0, _getNodeInterfaceRequest(file.proxy)),
            ZX.OK);
      }
      return file;
    }

    test('onOpen event on success', () async {
      var file =
          _createReadOnlyFile('test_str', openRightReadable | openFlagDescribe);

      await file.proxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    var expectedNodeAttrs = NodeAttributes(
        mode: modeTypeFile | modeProtectionMask,
        id: inoUnknown,
        contentSize: 0,
        storageSize: 0,
        linkCount: 1,
        creationTime: 0,
        modificationTime: 0);

    test('test getAttr', () async {
      var file = _createReadOnlyFile('test_str', openRightReadable);
      var response = await file.proxy.getAttr();
      expect(response.s, ZX.OK);
      expect(response.attributes, expectedNodeAttrs);
    });

    test('clone works', () async {
      var file = _createReadOnlyFile('test_str', openRightReadable);

      var clonedProxy = FileProxy();
      await file.proxy.clone(openRightReadable | openFlagDescribe,
          _getNodeInterfaceRequest(clonedProxy));

      await clonedProxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('clone works with POSIX compatibility', () async {
      var file = _createReadOnlyFile('test_str', openRightReadable);

      var clonedProxy = FileProxy();
      await file.proxy.clone(
          openRightReadable | openFlagDescribe | openFlagPosix,
          _getNodeInterfaceRequest(clonedProxy));

      await clonedProxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('clone fails when trying to pass Readable flag to Node Reference',
        () async {
      var file = _createReadOnlyFile(
          'test_str', openRightReadable | openFlagNodeReference);

      var clonedProxy = FileProxy();
      await file.proxy.clone(openRightReadable | openFlagDescribe,
          _getNodeInterfaceRequest(clonedProxy));

      await clonedProxy.onOpen.first.then((response) {
        expect(response.s, ZX.ERR_ACCESS_DENIED);
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('clone fails when trying to pass Writable flag to Node Reference',
        () async {
      var file = _createReadWriteFile('test_str',
          flags: openRightWritable | openFlagNodeReference);

      var clonedProxy = FileProxy();
      await file.proxy.clone(openRightWritable | openFlagDescribe,
          _getNodeInterfaceRequest(clonedProxy));

      await clonedProxy.onOpen.first.then((response) {
        expect(response.s, ZX.ERR_ACCESS_DENIED);
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('able to clone Node Reference', () async {
      var file = _createReadOnlyFile('test_str', openFlagNodeReference);

      var clonedProxy = FileProxy();
      await file.proxy.clone(openFlagNodeReference | openFlagDescribe,
          _getNodeInterfaceRequest(clonedProxy));

      await clonedProxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('clone should fail if requested rights exceed source rights',
        () async {
      var file = _createReadWriteFile('test_str', createProxy: false);
      var flagsToTest = [openRightReadable, openRightWritable];

      for (var flag in flagsToTest) {
        var proxy = FileProxy();
        expect(file.pseudoFile.connect(0, 0, _getNodeInterfaceRequest(proxy)),
            ZX.OK);

        var clonedProxy = FileProxy();
        await proxy.clone(
            flag | openFlagDescribe, _getNodeInterfaceRequest(clonedProxy));

        await clonedProxy.onOpen.first.then((response) {
          expect(response.s, ZX.ERR_ACCESS_DENIED);
          expect(response.info, isNull);
        }).catchError((err) async {
          fail(err.toString());
        });
      }
    });

    test('onOpen with describe flag', () async {
      var file =
          _createReadOnlyFile('test_str', openRightReadable | openFlagDescribe);

      await file.proxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('onOpen with NodeReference flag', () async {
      var file = _createReadOnlyFile(
          'test_str', openFlagNodeReference | openFlagDescribe);

      await file.proxy.onOpen.first.then((response) {
        expect(response.s, ZX.OK);
        expect(response.info, isNotNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('Directory not ignored with NodeReference flag', () async {
      var file = _createReadOnlyFile(
          'test_str',
          openFlagNodeReference | openFlagDescribe | openFlagDirectory,
          ZX.ERR_NOT_DIR);

      await file.proxy.onOpen.first.then((response) {
        expect(response.s, ZX.ERR_NOT_DIR);
        expect(response.info, isNull);
      }).catchError((err) async {
        fail(err.toString());
      });
    });

    test('GetAttr with NodeReference flag', () async {
      var file = _createReadOnlyFile('test_str', openFlagNodeReference);
      var response = await file.proxy.getAttr();
      expect(response.s, ZX.OK);
      expect(response.attributes, expectedNodeAttrs);
    });

    test('read file', () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);
      await _assertRead(file.proxy, 10, str);
    });

    test('read functions fails for NodeReference flag', () async {
      var file = _createReadOnlyFile(
          'test_str', openRightReadable | openFlagNodeReference);
      var readResponse = await file.proxy.read(1024);
      expect(readResponse.s, ZX.ERR_ACCESS_DENIED);

      var readAtResponse = await file.proxy.readAt(0, 1024);
      expect(readAtResponse.s, ZX.ERR_ACCESS_DENIED);
    });

    Future<void> _resetSeek(FileProxy proxy, [int? offset]) async {
      //reset seek
      var seekResponse =
          await proxy.seek(offset != null ? offset : 0, SeekOrigin.start);
      expect(seekResponse.s, ZX.OK);
    }

    test('simple write file', () async {
      var str = 'test_str';
      var file = _createReadWriteFile(str);
      var proxy = file.proxy;

      await _assertWrite(proxy, _newStrList);

      await _resetSeek(proxy);

      await _assertRead(file.proxy, 100, _newStr);

      await _assertFinalBuffer(file.proxy, file, str, _newStr);
    });

    test('readAt', () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      for (int i = 0; i < str.length; i++) {
        await _assertReadAt(file.proxy, 100, i, str.substring(i));
      }
    });

    test('read after readAt', () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      await _assertReadAt(file.proxy, 100, 1, str.substring(1));

      //readAt should not change seek
      await _assertRead(file.proxy, 100, str);
    });

    test('read should not affect readAt', () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      await _assertRead(file.proxy, 100, str);

      // after seek is changed, readAt should be able to read at any position
      await _assertReadAt(file.proxy, 100, 2, str.substring(2));
    });

    test('readAt should work for arbitrary length', () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      // try to read 3 chars
      await _assertReadAt(file.proxy, 3, 2, str.substring(2, 2 + 3));
    });

    test('read should work after readAt read arbitrary length', () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      await _assertReadAt(file.proxy, 3, 2, str.substring(2, 2 + 3));

      await _assertRead(file.proxy, 3, str.substring(0, 0 + 3));
    });

    test('readAt should fail for passing offset more than length of file',
        () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      await _assertReadAt(file.proxy, 100, str.length + 1, '',
          expectedStatus: ZX.ERR_OUT_OF_RANGE);
    });

    test('readAt should not fail for passing offset equal to length of file',
        () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      await _assertReadAt(file.proxy, 100, str.length, '');
    });

    test('read should not fail for reading from end of file', () async {
      var str = 'test_str';
      var file = _createReadOnlyFile(str, openRightReadable);

      await _resetSeek(file.proxy, str.length);
      await _assertRead(file.proxy, 100, '');
    });

    test('write file at arbitary position', () async {
      var str = 'test_str';
      var file = _createReadWriteFile(str);
      var proxy = file.proxy;

      await _resetSeek(proxy, 1);

      await _assertWrite(proxy, _newStrList);

      await _resetSeek(proxy);

      var expectedStr = str.substring(0, 1) + _newStr;
      await _assertRead(proxy, 100, expectedStr);

      await _assertFinalBuffer(proxy, file, str, expectedStr);
    });

    test('write should truncate if not enough capacity', () async {
      var str = 'test_str';
      var file = _createReadWriteFile(str);
      var proxy = file.proxy;

      await _resetSeek(proxy, 2);
      await _assertWrite(proxy, _newStrList, expectedSize: _newStr.length - 1);
      await _resetSeek(proxy);

      var expectedStr =
          str.substring(0, 2) + _newStr.substring(0, _newStr.length - 1);
      await _assertRead(proxy, 100, expectedStr);

      await _assertFinalBuffer(proxy, file, str, expectedStr);
    });

    test('write at end should fail', () async {
      var str = 'test_str';
      var file = _createReadWriteFile(str);
      var proxy = file.proxy;

      await _resetSeek(proxy, str.length);
      await _assertWrite(proxy, _newStrList,
          expectedStatus: ZX.ERR_OUT_OF_RANGE, expectedSize: 0);

      await _resetSeek(proxy);

      await _assertRead(proxy, 100, str);

      await _assertFinalBuffer(proxy, file, str, str);
    });

    group('write file when capacity is not initial lenght:', () {
      test('should not truncate if there is nough capacity', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str, capacity: str.length + 2);
        var proxy = file.proxy;

        await _resetSeek(proxy, 3);
        await _assertWrite(proxy, _newStrList);

        var expectedStr =
            str.substring(0, 3) + _newStr.substring(0, _newStr.length);
        await _assertReadAt(proxy, 100, 0, expectedStr);

        await _assertFinalBuffer(proxy, file, str, expectedStr);
      });

      test('write should fail when capacity is reached', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str, capacity: str.length + 2);
        var proxy = file.proxy;

        await _resetSeek(proxy, 3);
        await _assertWrite(proxy, _newStrList);

        var expectedStr =
            str.substring(0, 3) + _newStr.substring(0, _newStr.length);

        // write at end, should fail
        await _resetSeek(proxy, expectedStr.length);
        await _assertWrite(proxy, _newStrList,
            expectedStatus: ZX.ERR_OUT_OF_RANGE, expectedSize: 0);

        // no write should have happened
        await _assertReadAt(proxy, 100, 0, expectedStr);

        await _assertFinalBuffer(proxy, file, str, expectedStr);
      });

      test('write to end of initial len', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str, capacity: str.length + 2);
        var proxy = file.proxy;
        await _resetSeek(proxy, str.length);

        // write at end
        await _assertWrite(proxy, _newStrList, expectedSize: 2);

        var expectedStr = str + _newStr.substring(0, 2);
        await _assertReadAt(proxy, 100, 0, expectedStr);

        await _assertFinalBuffer(proxy, file, str, expectedStr);
      });

      test('writeAt should not depend on seek', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str, capacity: str.length + 2);
        var proxy = file.proxy;
        await _resetSeek(proxy, str.length);

        // seek is at end, write at 2nd position
        await _assertWriteAt(proxy, _newStrList, 2, _newStrList.length);

        // seek should not have changed
        var expectedStr = str.substring(0, 2) + _newStr;
        await _assertRead(proxy, 100, expectedStr.substring(str.length));

        await _assertFinalBuffer(proxy, file, str, expectedStr);
      });

      test('writeAt - end of file with', () async {
        var str = 'test_str';

        // writeAt should be able to write at end of the file
        var file = _createReadWriteFile(str, capacity: str.length + 5);
        var proxy = file.proxy;

        await _assertWriteAt(proxy, _newStrList, str.length, 5);

        var expectedStr = str + _newStr.substring(0, 5);
        await _assertRead(proxy, 100, expectedStr);

        await _assertFinalBuffer(proxy, file, str, expectedStr);
      });

      test('writeAt should fail for trying to write beyond end of file',
          () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str, capacity: str.length + 5);
        var proxy = file.proxy;

        await _assertWriteAt(proxy, _newStrList, str.length + 1, 0,
            expectedStatus: ZX.ERR_OUT_OF_RANGE);

        await _assertRead(proxy, 100, str);

        await _assertFinalBuffer(proxy, file, str, str);
      });

      test('write functions fails for NodeReference flag', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str,
            capacity: str.length + 5,
            flags: openRightWritable | openFlagNodeReference);
        var proxy = file.proxy;

        await _assertWriteAt(proxy, _newStrList, str.length + 1, 0,
            expectedStatus: ZX.ERR_ACCESS_DENIED);

        await _assertWrite(proxy, _newStrList,
            expectedSize: 0, expectedStatus: ZX.ERR_ACCESS_DENIED);
      });
    });

    test('various seek positions and reads', () async {
      var str = 'a very big string';
      var file = _createReadOnlyFile(str, openRightReadable);
      var proxy = file.proxy;

      var c = 5;
      var response = await proxy.read(5);
      expect(response.s, ZX.OK);
      expect(String.fromCharCodes(response.data), str.substring(0, c));

      var lastOffset = 5;
      c = 6;
      response = await proxy.read(6);
      expect(response.s, ZX.OK);
      expect(String.fromCharCodes(response.data),
          str.substring(lastOffset, lastOffset + c));

      c = 10;
      var responseSeek = await proxy.seek(c, SeekOrigin.start);
      expect(responseSeek.s, ZX.OK);
      expect(responseSeek.offset, c);
      response = await proxy.read(100);
      expect(response.s, ZX.OK);
      expect(String.fromCharCodes(response.data), str.substring(c));

      c = 2;
      responseSeek = await proxy.seek(c, SeekOrigin.start);
      expect(responseSeek.s, ZX.OK);
      expect(responseSeek.offset, c);
      lastOffset = c;
      c = 5;
      responseSeek = await proxy.seek(c, SeekOrigin.current);
      expect(responseSeek.s, ZX.OK);
      expect(responseSeek.offset, c + lastOffset);
      lastOffset = responseSeek.offset;
      response = await proxy.read(100);
      expect(response.s, ZX.OK);
      expect(String.fromCharCodes(response.data), str.substring(lastOffset));

      c = 0;
      responseSeek = await proxy.seek(c, SeekOrigin.end);
      expect(responseSeek.s, ZX.OK);
      expect(responseSeek.offset, str.length - 1);

      c = -3;
      responseSeek = await proxy.seek(c, SeekOrigin.end);
      expect(responseSeek.s, ZX.OK);
      expect(responseSeek.offset, str.length - 1 + c);
      response = await proxy.read(100);
      expect(response.s, ZX.OK);
      expect(String.fromCharCodes(response.data),
          str.substring(responseSeek.offset));

      // Check edge and error conditions
      c = 3;
      responseSeek = await proxy.seek(c, SeekOrigin.end);
      expect(responseSeek.s, ZX.ERR_OUT_OF_RANGE);

      c = -3;
      responseSeek = await proxy.seek(c, SeekOrigin.start);
      expect(responseSeek.s, ZX.ERR_OUT_OF_RANGE);

      c = 5;
      responseSeek = await proxy.seek(c, SeekOrigin.start);
      expect(responseSeek.s, ZX.OK);
      responseSeek = await proxy.seek(
          str.length - responseSeek.offset + 1, SeekOrigin.current);
      expect(responseSeek.s, ZX.ERR_OUT_OF_RANGE);
    });

    group('truncate: ', () {
      test('failure test', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str, createProxy: false);

        var proxy = FileProxy();
        expect(
            file.pseudoFile
                .connect(openRightReadable, 0, _getNodeInterfaceRequest(proxy)),
            ZX.OK);
        // truncate should fail
        var truncateResponse = await proxy.truncate(3);
        expect(truncateResponse, ZX.ERR_ACCESS_DENIED);

        await _assertRead(proxy, 100, str);

        await _assertFinalBuffer(proxy, file, str, str);
      });

      test('basic', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str);
        var proxy = file.proxy;

        var truncateResponse = await proxy.truncate(3);
        expect(truncateResponse, ZX.OK);
        var expectedStr = str.substring(0, 3);

        await _assertRead(proxy, 100, expectedStr);

        await _assertFinalBuffer(proxy, file, str, expectedStr);
      });

      test('on open', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str, createProxy: false);
        var proxy = FileProxy();
        expect(
            file.pseudoFile.connect(
                openRightReadable | openRightWritable | openFlagTruncate,
                0,
                _getNodeInterfaceRequest(proxy)),
            ZX.OK);

        await _assertFinalBuffer(proxy, file, str, '');
      });

      test('with seek', () async {
        var str = 'test_str';
        var file = _createReadWriteFile(str);
        var proxy = file.proxy;

        await _resetSeek(proxy, 5);
        // truncate should fail
        var truncateResponse = await proxy.truncate(3);
        expect(truncateResponse, ZX.OK);

        // seek and currentLen should be 3 so we should not get any data
        await _assertRead(proxy, 100, '');

        // truncate should have worked
        await _assertReadAt(proxy, 100, 0, str.substring(0, 3));
      });

      test('close should work', () async {
        var str = 'test_str';
        var file = _createReadOnlyFile(str, openRightReadable);
        // make sure file was opened
        file.pseudoFile.close();

        file.proxy.ctrl.whenClosed.asStream().listen(expectAsync1((_) {}));
      });
    });
  });
}

class _ReadOnlyFile {
  late PseudoFile pseudoFile;
  late FileProxy proxy;
}

class _ReadWriteFile {
  String? buffer;
  late PseudoFile pseudoFile;
  late FileProxy proxy;
}
