// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:isolate';
import 'dart:ui';

import 'package:litetest/litetest.dart';

void main() {
  test('PlatformIsolate.isRunningOnPlatformThread, false cases', () async {
    expect(PlatformIsolate.isRunningOnPlatformThread(), isFalse);

    final isPlatThread =
        await Isolate.run(() => PlatformIsolate.isRunningOnPlatformThread());
    expect(isPlatThread, isFalse);
  });

  test('PlatformIsolate.spawn', () async {
    final resultCompleter = Completer();
    final resultPort =
        RawReceivePort((message) => resultCompleter.complete(message));
    final isolate = await PlatformIsolate.spawn(
        (port) => port.send(PlatformIsolate.isRunningOnPlatformThread()),
        resultPort.sendPort);
    expect(await resultCompleter.future, isTrue);
    resultPort.close();
  });

  test('PlatformIsolate.run', () async {
    final isPlatThread = await PlatformIsolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread());
    expect(isPlatThread, isTrue);
  });

  test('PlatformIsolate.spawn, async operations', () async {
    final resultCompleter = Completer();
    final resultPort =
        RawReceivePort((message) => resultCompleter.complete(message));
    final isolate = await PlatformIsolate.spawn((port) async {
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
      port.send(PlatformIsolate.isRunningOnPlatformThread());
    }, resultPort.sendPort);
    expect(await resultCompleter.future, isTrue);
    resultPort.close();
  });

  test('PlatformIsolate.run, async operations', () async {
    final isPlatThread = await PlatformIsolate.run(() async {
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
      return PlatformIsolate.isRunningOnPlatformThread();
    });
    expect(isPlatThread, isTrue);
  });

  test('PlatformIsolate.spawn, send and receive messages', () async {
    // Send numbers 1 to 10 to the platform isolate. The platform isolate
    // multiplies them by 100 and sends them back.
    final completer = Completer();
    int sum = 0;
    final recvPort = RawReceivePort((message) {
      if (message is SendPort) {
        for (int i = 1; i <= 10; ++i) {
          message.send(i);
        }
      } else {
        sum += message as int;
        if (message == 1000) {
          completer.complete();
        }
      }
    });
    final isolate = await PlatformIsolate.spawn((port) {
      final recvPort = RawReceivePort();
      recvPort.handler = (message) {
        port.send((message as int) * 100);
        if (message == 10) {
          recvPort.close();
        }
      };
      port.send(recvPort.sendPort);
    }, recvPort.sendPort);
    await completer.future;
    expect(sum, 5500); // sum(1 to 10) * 100
    recvPort.close();
  });

  test('PlatformIsolate.run, send and receive messages', () async {
    // Send numbers 1 to 10 to the platform isolate. The platform isolate
    // multiplies them by 100 and sends them back.
    int sum = 0;
    final recvPort = RawReceivePort((message) {
      if (message is SendPort) {
        for (int i = 1; i <= 10; ++i) {
          message.send(i);
        }
      } else {
        sum += message as int;
      }
    });
    final sendPort = recvPort.sendPort;
    final isolate = await PlatformIsolate.run(() async {
      final completer = Completer();
      final recvPort = RawReceivePort((message) {
        sendPort.send((message as int) * 100);
        if (message == 10) {
          completer.complete();
        }
      });
      sendPort.send(recvPort.sendPort);
      await completer.future;
      recvPort.close();
    });
    expect(sum, 5500); // sum(1 to 10) * 100
    recvPort.close();
  });

  test('PlatformIsolate.spawn, throws', () async {
    // Like Isolate.spawn, this should log the unhandled exception, but continue
    // running, and the test should pass anyway.
    await PlatformIsolate.spawn((_) => throw "Oh no!", null);
  });

  test('PlatformIsolate.spawn, onError', () async {
    final errorCompleter = Completer();
    final onErrorPort = RawReceivePort();
    onErrorPort.handler = (msg) {
      errorCompleter.complete(msg[0]);
      onErrorPort.close();
    };
    await PlatformIsolate.spawn((_) => throw "Oh no!", null,
        onError: onErrorPort.sendPort);
    expect(await errorCompleter.future, "Oh no!");
  });

  test('PlatformIsolate.run, throws', () async {
    bool throws = false;
    try {
      await PlatformIsolate.run(() => throw "Oh no!");
    } catch (error) {
      expect(error, "Oh no!");
      throws = true;
    }
    expect(throws, true);
  });

  test('PlatformIsolate.run, async throws', () async {
    bool throws = false;
    try {
      await PlatformIsolate.run(() async {
        await Future.delayed(Duration(milliseconds: 100));
        await Future.delayed(Duration(milliseconds: 100));
        await Future.delayed(Duration(milliseconds: 100));
        throw "Oh no!";
      });
    } catch (error) {
      expect(error, "Oh no!");
      throws = true;
    }
    expect(throws, true);
  });

  test('PlatformIsolate.spawn, onExit', () async {
    final exitCompleter = Completer();
    final onExitPort = RawReceivePort();
    onExitPort.handler = (_) {
      exitCompleter.complete(true);
      onExitPort.close();
    };
    await PlatformIsolate.spawn((_) async {
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
    }, null, onExit: onExitPort.sendPort);
    expect(await exitCompleter.future, true);
  });

  test('PlatformIsolate.spawn, onExit throws', () async {
    final exitCompleter = Completer();
    final onExitPort = RawReceivePort();
    onExitPort.handler = (_) {
      exitCompleter.complete(true);
      onExitPort.close();
    };
    await PlatformIsolate.spawn((_) async {
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
      await Future.delayed(Duration(milliseconds: 100));
      throw "Oh no!";
    }, null, onExit: onExitPort.sendPort);
    expect(await exitCompleter.future, true);
  });

  test('PlatformIsolate.spawn, errors are fatal', () async {
    // Send two messages to the platform isolate. The first causes it to throw
    // an error, which causes the isolate to exit because errorsAreFatal is set
    // to true. So the second should not be received.
    bool secondMessageReplyReceived = false;
    final recvPort = RawReceivePort((message) {
      if (message is SendPort) {
        message.send("First");
        message.send("Second");
      } else {
        secondMessageReplyReceived = true;
      }
    });

    final exitCompleter = Completer();
    final exitPort = RawReceivePort();
    exitPort.handler = (_) {
      exitCompleter.complete();
      exitPort.close();
    };

    final isolate = await PlatformIsolate.spawn((port) {
      final recvPort = RawReceivePort();
      recvPort.handler = (message) {
        if (message == "First") {
          throw "Oh no!";
        } else {
          port.send("Reply to second message");
          recvPort.close();
        }
      };
      port.send(recvPort.sendPort);
    }, recvPort.sendPort, onExit: exitPort.sendPort, errorsAreFatal: true);

    await exitCompleter.future;
    expect(secondMessageReplyReceived, false);
    recvPort.close();
  });

  test('PlatformIsolate.spawn, errors are not fatal', () async {
    // Send two messages to the platform isolate. The first causes it to throw
    // an error, but the isolate continues running because errorsAreFatal is set
    // to false. So the second should be received.
    bool secondMessageReplyReceived = false;
    final recvPort = RawReceivePort((message) {
      if (message is SendPort) {
        message.send("First");
        message.send("Second");
      } else {
        secondMessageReplyReceived = true;
      }
    });

    final exitCompleter = Completer();
    final exitPort = RawReceivePort();
    exitPort.handler = (_) {
      exitCompleter.complete();
      exitPort.close();
    };

    final isolate = await PlatformIsolate.spawn((port) {
      final recvPort = RawReceivePort();
      recvPort.handler = (message) {
        if (message == "First") {
          throw "Oh no!";
        } else {
          port.send("Reply to second message");
          recvPort.close();
        }
      };
      port.send(recvPort.sendPort);
    }, recvPort.sendPort, onExit: exitPort.sendPort, errorsAreFatal: false);

    await exitCompleter.future;
    expect(secondMessageReplyReceived, true);
    recvPort.close();
  });

  test('PlatformIsolate.spawn, root isolate only', () async {
    await Isolate.run(() {
      expect(() => PlatformIsolate.spawn((_) => print('Unreachable'), null),
          throws);
    });
  });

  test('PlatformIsolate.run, root isolate only', () async {
    await Isolate.run(() {
      expect(() => PlatformIsolate.run(() => print('Unreachable')), throws);
    });
  });

  // TODO(flutter/flutter#136314): At the moment this works, but logs a spurious
  // error. We need to silence that error.
  /*test('PlatformIsolate.spawn, onExit, self exit', () async {
    final exitCompleter = Completer();
    final onExitPort = RawReceivePort();
    onExitPort.handler = (_) {
      exitCompleter.complete(true);
      onExitPort.close();
    };
    await PlatformIsolate.spawn((_) async {
        Isolate.exit();
      }, null, onExit: onExitPort.sendPort);
    expect(await exitCompleter.future, true);
  });*/

  // TODO(flutter/flutter#136314): Support pausing/resuming PlatformIsolates.
  /*test('PlatformIsolate.spawn, start paused', () async {
    // Start 2 platform isolates, one paused and one not. The paused isolate
    // sends a message to port1, which will throw when it gets any message. The
    // second isolate sends a message to port2, which swaps out the handler of
    // port1 so that it doesn't throw, then unpauses the first isolate. Since
    // the first isolate is started paused, the message won't be sent to port1
    // until the handler has been swapped out.
    RawReceivePort port1 = RawReceivePort((_) => throw "NOT YET");

    final isolate1 = await PlatformIsolate.spawn(
        (port) => port.send("PlatIso1"), port1.sendPort, errorsAreFatal: true);

    final completer = Completer();
    final port2 = RawReceivePort();
    port2.handler = (message) {
      Expect.equals("PlatIso2", message);
      port2.close();
      port1.handler = (message) {
        Expect.equals("PlatIso1", message);
        port1.close();
        completer.complete();
      };
      print("Resume isolate1");
      isolate1.resume(isolate1.pauseCapability!);
    };

    await PlatformIsolate.spawn(
        (port) => port.send("PlatIso2"), port2.sendPort, errorsAreFatal:false);
    await completer.future;
  });*/
}
