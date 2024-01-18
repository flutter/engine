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

    final isPlatThread = await Isolate.run(
        () => PlatformIsolate.isRunningOnPlatformThread());
    expect(isPlatThread, isFalse);
  });

  test('PlatformIsolate.spawn', () async {
    final resultCompleter = Completer();
    final resultPort = RawReceivePort();
    resultPort.handler = (message) => resultCompleter.complete(message);
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
    final resultPort = RawReceivePort();
    resultPort.handler = (message) => resultCompleter.complete(message);
    final isolate = await PlatformIsolate.spawn(
        (port) async {
          await Future.delayed(Duration(milliseconds: 100));
          await Future.delayed(Duration(milliseconds: 100));
          await Future.delayed(Duration(milliseconds: 100));
          port.send(PlatformIsolate.isRunningOnPlatformThread());
        },
        resultPort.sendPort);
    expect(await resultCompleter.future, isTrue);
    resultPort.close();
  });

  test('PlatformIsolate.run, async operations', () async {
    final isPlatThread = await PlatformIsolate.run(
        () async {
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
    final recvPort = RawReceivePort();
    int sum = 0;
    recvPort.handler = (message) {
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
    };
    final isolate = await PlatformIsolate.spawn(
        (port) {
          final recvPort = RawReceivePort();
          recvPort.handler = (message) {
            port.send((message as int) * 100);
            if (message == 10) {
              recvPort.close();
            }
          };
          port.send(recvPort.sendPort);
        },
        recvPort.sendPort);
    await completer.future;
    expect(sum, 5500);  // sum(1 to 10) * 100
    recvPort.close();
  });

  test('PlatformIsolate.run, send and receive messages', () async {
    // Send numbers 1 to 10 to the platform isolate. The platform isolate
    // multiplies them by 100 and sends them back.
    final recvPort = RawReceivePort();
    int sum = 0;
    recvPort.handler = (message) {
      if (message is SendPort) {
        for (int i = 1; i <= 10; ++i) {
          message.send(i);
        }
      } else {
        sum += message as int;
      }
    };
    final sendPort = recvPort.sendPort;
    final isolate = await PlatformIsolate.run(
        () async {
          final completer = Completer();
          final recvPort = RawReceivePort();
          recvPort.handler = (message) {
            sendPort.send((message as int) * 100);
            if (message == 10) {
              completer.complete();
            }
          };
          sendPort.send(recvPort.sendPort);
          await completer.future;
          recvPort.close();
        });
    expect(sum, 5500);  // sum(1 to 10) * 100
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

  /*test('PlatformIsolate.spawn, errors not fatal', () async {
    // Same as the 'send and receive messages' test, but we also send strings,
    // which cause the 'as int' expression in the platform isolate to throw. The
    // test should still complete successfully because we set errorsAreFatal to
    // false.
    final completer = Completer();
    final recvPort = RawReceivePort();
    int sum = 0;
    recvPort.handler = (message) {
      if (message is SendPort) {
        for (int i = 1; i <= 10; ++i) {
          message.send(i);
          message.send("Throw me: $i");
        }
      } else {
        sum += message as int;
        if (message == 1000) {
          completer.complete();
        }
      }
    };
    final isolate = await PlatformIsolate.spawn(
        (port) {
          final recvPort = RawReceivePort();
          recvPort.handler = (message) {
            port.send((message as int) * 100);
            if (message == 10) {
              recvPort.close();
            }
          };
          port.send(recvPort.sendPort);
        },
        recvPort.sendPort);
    await completer.future;
    expect(sum, 5500);  // sum(1 to 10) * 100
    recvPort.close();
  });*/

  test('PlatformIsolate.spawn, root isolate only', () async {
    await Isolate.run(
        () {
          expect(() => PlatformIsolate.spawn((_) => print('Unreachable'), null),
              throws);
        });
  });

  test('PlatformIsolate.run, root isolate only', () async {
    await Isolate.run(
        () {
          expect(() => PlatformIsolate.run(() => print('Unreachable')), throws);
        });
  });
}
