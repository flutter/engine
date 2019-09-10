import 'dart:ui' as ui;
import 'dart:typed_data';
import 'dart:convert';

import 'package:test/test.dart';

void main() {

  test('push pop', () async {
    String channel = "foo";
    var list = utf8.encode('bar');
    var buffer = list is Uint8List ? list.buffer : new Uint8List.fromList(list).buffer;
    ByteData data = ByteData.view(buffer);
    ui.ChannelBuffers buffers = ui.ChannelBuffers();
    ui.PlatformMessageResponseCallback callback = (ByteData responseData) {};
    buffers.push(channel, data, callback);
    ui.StoredMessage storedMessage = buffers.pop(channel);
    expect(storedMessage.data, equals(data));
    expect(storedMessage.callback, equals(callback));
  });

  test('empty', () async {
    String channel = "foo";
    var list = utf8.encode('bar');
    var buffer = list is Uint8List ? list.buffer : new Uint8List.fromList(list).buffer;
    ByteData data = ByteData.view(buffer);
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

  test('ringbuffer push pop', () async {
    ui.RingBuffer<int> ringBuffer = ui.RingBuffer<int>(10);
    ringBuffer.push(1);
    expect(ringBuffer.pop(), equals(1));
  });

  test('ringbuffer overflow', () async {
    ui.RingBuffer<int> ringBuffer = ui.RingBuffer<int>(3);
    expect(ringBuffer.push(1), equals(false));
    expect(ringBuffer.push(2), equals(false));
    expect(ringBuffer.push(3), equals(false));
    expect(ringBuffer.push(4), equals(true));
    expect(ringBuffer.pop(), equals(2));
  });

  test('ringbuffer length', () async {
    ui.RingBuffer<int> ringBuffer = ui.RingBuffer<int>(3);
    expect(ringBuffer.length, equals(0));
    ringBuffer.push(123);
    expect(ringBuffer.length, equals(1));
  });

  test('ringbuffer underflow', () async {
    ui.RingBuffer<int> ringBuffer = ui.RingBuffer<int>(3);
    expect(ringBuffer.pop(), equals(null));
  });
}
