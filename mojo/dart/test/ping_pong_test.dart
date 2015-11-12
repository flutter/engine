// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:mojo/core.dart' as core;

ByteData byteDataOfString(String s) {
  return new ByteData.view((new Uint8List.fromList(s.codeUnits)).buffer);
}

String stringOfByteData(ByteData bytes) {
  return new String.fromCharCodes(bytes.buffer.asUint8List().toList());
}

void expectStringFromEndpoint(
    String expected, core.MojoMessagePipeEndpoint endpoint) {
  // Query how many bytes are available.
  var result = endpoint.query();
  assert(result != null);
  int size = result.bytesRead;
  assert(size > 0);

  // Read the data.
  ByteData bytes = new ByteData(size);
  result = endpoint.read(bytes);
  assert(result != null);
  assert(size == result.bytesRead);

  // Convert to a string and check.
  String msg = stringOfByteData(bytes);
  assert(expected == msg);
}

void pipeTestIsolate(core.MojoMessagePipeEndpoint endpoint) {
  var eventSubscription = new core.MojoEventSubscription(endpoint.handle);
  eventSubscription.subscribe((List<int> event) {
    var mojoSignals = new core.MojoHandleSignals(event[1]);
    if (mojoSignals.isReadWrite) {
      throw 'We should only be reading or writing, not both.';
    } else if (mojoSignals.isReadable) {
      expectStringFromEndpoint("Ping", endpoint);
      eventSubscription.enableWriteEvents();
    } else if (mojoSignals.isWritable) {
      endpoint.write(byteDataOfString("Pong"));
      eventSubscription.enableReadEvents();
    } else if (mojoSignals.isPeerClosed) {
      eventSubscription.close();
    } else {
      throw 'Unexpected event.';
    }
  });
}

main() {
  var pipe = new core.MojoMessagePipe();
  var endpoint = pipe.endpoints[0];
  var eventSubscription = new core.MojoEventSubscription(endpoint.handle);
  Isolate.spawn(pipeTestIsolate, pipe.endpoints[1]).then((_) {
    eventSubscription.subscribe((List<int> event) {
      var mojoSignals = new core.MojoHandleSignals(event[1]);
      if (mojoSignals.isReadWrite) {
        throw 'We should only be reading or writing, not both.';
      } else if (mojoSignals.isReadable) {
        expectStringFromEndpoint("Pong", endpoint);
        eventSubscription.close();
      } else if (mojoSignals.isWritable) {
        endpoint.write(byteDataOfString("Ping"));
        eventSubscription.enableReadEvents();
      } else if (mojoSignals.isPeerClosed) {
        throw 'This end should close first.';
      } else {
        throw 'Unexpected event.';
      }
    });
    eventSubscription.enableWriteEvents();
  });
}
