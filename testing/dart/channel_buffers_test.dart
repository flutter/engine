import 'dart:ui' as ui;
import 'dart:typed_data';
import 'dart:convert';

import 'package:test/test.dart';

void main() {

  ByteData _makeByteData(String str) {
    var list = utf8.encode(str);
    var buffer = list is Uint8List ? list.buffer : new Uint8List.fromList(list).buffer;
    return ByteData.view(buffer);
  }

  String _getString(ByteData data) {
    final buffer = data.buffer;
    var list = buffer.asUint8List(data.offsetInBytes, data.lengthInBytes);
    return utf8.decode(list);
  }

  test('push pop', () async {
    String channel = "foo";
    ByteData data = _makeByteData('bar');
    ui.ChannelBuffers buffers = ui.ChannelBuffers();
    ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    buffers.push(channel, data, callback);
    ui.StoredMessage storedMessage = buffers.pop(channel);
    expect(storedMessage.data, equals(data));
    expect(storedMessage.callback, equals(callback));
  });

  test('empty', () async {
    String channel = "foo";
    ByteData data = _makeByteData('bar');
    ui.ChannelBuffers buffers = ui.ChannelBuffers();
    ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    expect(buffers.isEmpty(channel), equals(true));
    buffers.push(channel, data, callback);
    expect(buffers.isEmpty(channel), equals(false));
  });

  test('pop', () async {
    ui.ChannelBuffers buffers = ui.ChannelBuffers();
    expect(buffers.pop("channel"), equals(null));
  });

  test('overflow', () async {
    String channel = "foo";
    ByteData one = _makeByteData('one');
    ByteData two = _makeByteData('two');
    ByteData three = _makeByteData('three');
    ByteData four = _makeByteData('four');
    ui.ChannelBuffers buffers = ui.ChannelBuffers();
    ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    buffers.resize(channel, 3);
    expect(buffers.push(channel, one, callback), equals(false));
    expect(buffers.push(channel, two, callback), equals(false));
    expect(buffers.push(channel, three, callback), equals(false));
    expect(buffers.push(channel, four, callback), equals(true));
    expect(_getString(buffers.pop(channel).data), equals('two'));
  });
}
