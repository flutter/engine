// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:meta/meta.dart';
import 'package:zircon/zircon.dart';

import 'codec.dart';
import 'error.dart';
import 'types.dart';
// ignore_for_file: public_member_api_docs

const int kMessageHeaderSize = 16;
const int kMessageTxidOffset = 0;
const int kMessageFlagOffset = 4;
const int kMessageMagicOffset = 7;
const int kMessageOrdinalOffset = 8;

const int kMagicNumberInitial = 1;

class _BaseMessage {
  _BaseMessage(this.data, this.handles);

  final ByteData data;
  final List<Handle> handles;

  int get txid => data.getUint32(kMessageTxidOffset, Endian.little);
  int get ordinal => data.getUint64(kMessageOrdinalOffset, Endian.little);
  int get magic => data.getUint8(kMessageMagicOffset);

  bool isCompatible() => magic == kMagicNumberInitial;

  void hexDump() {
    const int width = 16;
    Uint8List list = Uint8List.view(data.buffer, 0);
    StringBuffer buffer = StringBuffer();
    final RegExp isPrintable = RegExp(r'\w');
    for (int i = 0; i < data.lengthInBytes; i += width) {
      StringBuffer hex = StringBuffer();
      StringBuffer printable = StringBuffer();
      for (int j = 0; j < width && i + j < data.lengthInBytes; j++) {
        int v = list[i + j];
        String s = v.toRadixString(16);
        if (s.length == 1)
          hex.write('0$s ');
        else
          hex.write('$s ');

        s = String.fromCharCode(v);
        if (isPrintable.hasMatch(s)) {
          printable.write(s);
        } else {
          printable.write('.');
        }
      }
      buffer.write('${hex.toString().padRight(3 * width)} $printable\n');
    }

    print('==================================================\n'
        '$buffer'
        '==================================================');
  }

  void closeHandles() {
    for (int i = 0; i < handles.length; ++i) {
      handles[i].close();
    }
  }

  @override
  String toString() {
    return 'Message(numBytes=${data.lengthInBytes}, numHandles=${handles.length})';
  }
}

class IncomingMessage extends _BaseMessage {
  IncomingMessage(data, handles) : super(data, handles);
  IncomingMessage.fromReadResult(ReadResult result)
      : assert(result.status == ZX.OK),
        super(result.bytes, result.handles);
  @visibleForTesting
  IncomingMessage.fromOutgoingMessage(OutgoingMessage outgoingMessage)
      : super(outgoingMessage.data, outgoingMessage.handles);
}

class OutgoingMessage extends _BaseMessage {
  OutgoingMessage(data, handles) : super(data, handles);

  set txid(int value) =>
      data.setUint32(kMessageTxidOffset, value, Endian.little);
}

/// Encodes a FIDL message that contains a single parameter.
void encodeMessage<T>(
    Encoder encoder, int inlineSize, MemberType typ, T value) {
  encoder.alloc(inlineSize, 0);
  typ.encode(encoder, value, kMessageHeaderSize, 1);
}

/// Encodes a FIDL message with multiple parameters.  The callback parameter
/// provides a decoder that is initialized on the provided Message, which
/// callers can use to decode specific types.  This functionality (encoding
/// multiple parameters) is implemented using a callback because each call to
/// MemberType.encode() must pass in a concrete type, rather than an element
/// popped from a List<FidlType>.
void encodeMessageWithCallback(Encoder encoder, int inlineSize, Function() f) {
  encoder.alloc(inlineSize, 0);
  f();
}

void _validateDecoding(Decoder decoder) {
  if (decoder.countUnclaimedHandles() > 0) {
    // If there are unclaimed handles at the end of the decoding, close all
    // handles to the best of our ability, and throw an error.
    for (var handle in decoder.handles) {
      try {
        handle.close();
        // ignore: avoid_catches_without_on_clauses
      } catch (e) {
        // best effort
      }
    }

    int unclaimed = decoder.countUnclaimedHandles();
    int total = decoder.handles.length;
    throw FidlError(
        'Message contains extra handles (unclaimed: $unclaimed, total: $total)',
        FidlErrorCode.fidlTooManyHandles);
  }
}

/// Decodes a FIDL message that contains a single parameter.
T decodeMessage<T>(IncomingMessage message, int inlineSize, MemberType typ) {
  final Decoder decoder = Decoder(message)
    ..claimMemory(kMessageHeaderSize + inlineSize, 0);
  T decoded = typ.decode(decoder, kMessageHeaderSize, 1);
  _validateDecoding(decoder);
  return decoded;
}

/// Decodes a FIDL message with multiple parameters.  The callback parameter
/// provides a decoder that is initialized on the provided Message, which
/// callers can use to decode specific types.  The return result of the callback
/// (e.g. the decoded parameters, wrapped in a containing class/struct) is
/// returned as the result of the function.  This functionality (decoding
/// multiple parameters) is implemented using a callback because returning a
/// list would be insufficient: the list would be of type List<FidlType>,
/// whereas we want to retain concrete types of each decoded parameter.  The
/// only way to accomplish this in Dart is to pass in a function that collects
/// these multiple values into a bespoke, properly typed class.
A decodeMessageWithCallback<A>(
    IncomingMessage message, int inlineSize, A Function(Decoder decoder) f) {
  final Decoder decoder = Decoder(message)
    ..claimMemory(kMessageHeaderSize + inlineSize, 0);
  A out = f(decoder);
  _validateDecoding(decoder);
  return out;
}

typedef IncomingMessageSink = void Function(IncomingMessage message);
typedef OutgoingMessageSink = void Function(OutgoingMessage message);
