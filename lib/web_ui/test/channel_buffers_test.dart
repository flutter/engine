// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6

import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

void main() {

  ByteData _makeByteData(String str) {
    final Uint8List list = utf8.encode(str) as Uint8List;
    final ByteBuffer buffer = list is Uint8List ? list.buffer : Uint8List.fromList(list).buffer;
    return ByteData.view(buffer);
  }

  void _resize(ui.ChannelBuffers buffers, String name, int newSize) {
    buffers.handleMessage(_makeByteData('resize\r$name\r$newSize'));
  }

  test('push drain', () async {
    const String channel = 'foo';
    final ByteData data = _makeByteData('bar');
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    final ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    buffers.push(channel, data, callback);
    await buffers.drain(channel, (ByteData drainedData, ui.PlatformMessageResponseCallback drainedCallback) {
      expect(drainedData, equals(data));
      expect(drainedCallback, equals(callback));
      return;
    });
  });

  test('deprecated drain is sync', () async {
    const String channel = 'foo';
    final ByteData data = _makeByteData('message');
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    final ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    buffers.push(channel, data, callback);
    final List<String> log = <String>[];
    final Completer<void> completer = Completer<void>();
    scheduleMicrotask(() { log.add('before drain, microtask'); });
    log.add('before drain');
    buffers.drain(channel, (ByteData drainedData, ui.PlatformMessageResponseCallback drainedCallback) async {
      log.add('callback');
      completer.complete();
    });
    log.add('after drain, before await');
    await completer.future;
    log.add('after await');
    expect(log, <String>[
      'before drain',
      'callback',
      'after drain, before await',
      'before drain, microtask',
      'after await'
    ]);
  });

  test('push drain zero', () async {
    const String channel = 'foo';
    final ByteData data = _makeByteData('bar');
    final
    ui.ChannelBuffers buffers = EngineChannelBuffers();
    final ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    _resize(buffers, channel, 0);
    buffers.push(channel, data, callback);
    bool didCall = false;
    await buffers.drain(channel, (ByteData drainedData, ui.PlatformMessageResponseCallback drainedCallback) {
      didCall = true;
      return;
    });
    expect(didCall, equals(false));
  });

  test('empty', () async {
    const String channel = 'foo';
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    bool didCall = false;
    await buffers.drain(channel, (ByteData drainedData, ui.PlatformMessageResponseCallback drainedCallback) {
      didCall = true;
      return;
    });
    expect(didCall, equals(false));
  });

  test('overflow', () async {
    const String channel = 'foo';
    final ByteData one = _makeByteData('one');
    final ByteData two = _makeByteData('two');
    final ByteData three = _makeByteData('three');
    final ByteData four = _makeByteData('four');
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    final ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    _resize(buffers, channel, 3);
    expect(buffers.push(channel, one, callback), equals(false));
    expect(buffers.push(channel, two, callback), equals(false));
    expect(buffers.push(channel, three, callback), equals(false));
    expect(buffers.push(channel, four, callback), equals(true));
    int counter = 0;
    await buffers.drain(channel, (ByteData drainedData, ui.PlatformMessageResponseCallback drainedCallback) {
      if (counter++ == 0) {
        expect(drainedData, equals(two));
        expect(drainedCallback, equals(callback));
      }
      return;
    });
    expect(counter, equals(3));
  });

  test('resize drop', () async {
    const String channel = 'foo';
    final ByteData one = _makeByteData('one');
    final ByteData two = _makeByteData('two');
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    _resize(buffers, channel, 100);
    final ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    expect(buffers.push(channel, one, callback), equals(false));
    expect(buffers.push(channel, two, callback), equals(false));
    _resize(buffers, channel, 1);
    int counter = 0;
    await buffers.drain(channel, (ByteData drainedData, ui.PlatformMessageResponseCallback drainedCallback) {
      if (counter++ == 0) {
        expect(drainedData, equals(two));
        expect(drainedCallback, equals(callback));
      }
      return;
    });
    expect(counter, equals(1));
  });

  test('resize dropping calls callback', () async {
    const String channel = 'foo';
    final ByteData one = _makeByteData('one');
    final ByteData two = _makeByteData('two');
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    bool didCallCallback = false;
    final ui.PlatformMessageResponseCallback oneCallback = (ByteData responseData) {
      didCallCallback = true;
    };
    final ui.PlatformMessageResponseCallback twoCallback = (ByteData responseData) {};
    _resize(buffers, channel, 100);
    expect(buffers.push(channel, one, oneCallback), equals(false));
    expect(buffers.push(channel, two, twoCallback), equals(false));
    _resize(buffers, channel, 1);
    expect(didCallCallback, equals(true));
  });

  test('overflow calls callback', () async {
    const String channel = 'foo';
    final ByteData one = _makeByteData('one');
    final ByteData two = _makeByteData('two');
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    bool didCallCallback = false;
    final ui.PlatformMessageResponseCallback oneCallback = (ByteData responseData) {
      didCallCallback = true;
    };
    final ui.PlatformMessageResponseCallback twoCallback = (ByteData responseData) {};
    _resize(buffers, channel, 1);
    expect(buffers.push(channel, one, oneCallback), equals(false));
    expect(buffers.push(channel, two, twoCallback), equals(true));
    expect(didCallCallback, equals(true));
  });

  test('handle garbage', () async {
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    expect(() => buffers.handleMessage(_makeByteData('asdfasdf')),
           throwsException);
  });

  test('handle resize garbage', () async {
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    expect(() => buffers.handleMessage(_makeByteData('resize\rfoo\rbar')),
           throwsException);
  });

  test('ChannelBuffers.setListener', () async {
    final List<String> log = <String>[];
    final ui.ChannelBuffers buffers = EngineChannelBuffers();
    final ByteData one = _makeByteData('one');
    final ByteData two = _makeByteData('two');
    final ByteData three = _makeByteData('three');
    final ByteData four = _makeByteData('four');
    final ByteData five = _makeByteData('five');
    final ByteData six = _makeByteData('six');
    final ByteData seven = _makeByteData('seven');
    buffers.push('a', one, (ByteData data) { });
    buffers.push('b', two, (ByteData data) { });
    buffers.push('a', three, (ByteData data) { });
    log.add('top');
    buffers.setListener('a', (ByteData data, ui.PlatformMessageResponseCallback callback) {
      log.add('a1: ${utf8.decode(data.buffer.asUint8List())}');
    });
    log.add('-1');
    await null;
    log.add('-2');
    buffers.setListener('a', (ByteData data, ui.PlatformMessageResponseCallback callback) {
      log.add('a2: ${utf8.decode(data.buffer.asUint8List())}');
    });
    log.add('-3');
    await null;
    log.add('-4');
    buffers.setListener('b', (ByteData data, ui.PlatformMessageResponseCallback callback) {
      log.add('b: ${utf8.decode(data.buffer.asUint8List())}');
    });
    log.add('-5');
    await null; // first microtask after setting listener drains the first message
    await null; // second microtask ends the draining.
    log.add('-6');
    buffers.push('b', four, (ByteData data) { });
    buffers.push('a', five, (ByteData data) { });
    log.add('-7');
    await null;
    log.add('-8');
    buffers.clearListener('a');
    buffers.push('a', six, (ByteData data) { });
    buffers.push('b', seven, (ByteData data) { });
    await null;
    log.add('-9');
    expect(log, <String>[
      'top',
      '-1',
      'a1: three',
      '-2',
      '-3',
      '-4',
      '-5',
      'b: two',
      '-6',
      'b: four',
      'a2: five',
      '-7',
      '-8',
      'b: seven',
      '-9',
    ]);
  });
}
