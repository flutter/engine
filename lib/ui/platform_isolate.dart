// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

/// Methods for running code in [Isolate]s that run on the Flutter platform
/// thread.
///
/// This is an experimental API. It may be changed or removed in future versions
/// based on user feedback.
abstract final class PlatformIsolate {
  static Future<Isolate> _spawn<T>(void Function(T) entryPoint, T message,
      {SendPort? onExit}) {
    final Completer<Isolate> isolateCompleter = Completer<Isolate>();
    final RawReceivePort isolateReadyPort = _receiveOne((Object readyMessage) {
      if (readyMessage is _PlatformIsolateReadyMessage) {
        final Isolate isolate = Isolate(readyMessage.controlPort,
            pauseCapability: readyMessage.pauseCapability,
            terminateCapability: readyMessage.terminateCapability);
        if (onExit != null) {
          isolate.addOnExitListener(onExit);
        }

        isolateCompleter.complete(isolate);
        readyMessage.entryPointPort.send((entryPoint, message));
      } else if (readyMessage is String) {
        // We encountered an error while starting the new isolate.
        isolateCompleter.completeError(
            IsolateSpawnException('Unable to spawn isolate: $readyMessage'));
      } else {
        // This shouldn't happen.
        isolateCompleter.completeError(IsolateSpawnException(
            'Internal error: unexpected format for ready message: '
            "'$readyMessage'"));
      }
    });
    try {
      _nativeSpawn(_platformIsolateMain<T>, isolateReadyPort.sendPort,
          'PlatformIsolate');
    } on Object {
      isolateReadyPort.close();
      rethrow;
    }
    return isolateCompleter.future;
  }

  static void _platformIsolateMain<T>(SendPort isolateReadyPort) {
    final RawReceivePort entryPointPort = _receiveOne(
        ((void Function(T), T) entryPointAndMessage) {
      final (void Function(T) entryPoint, T message) = entryPointAndMessage;
      entryPoint(message);
    });
    final Isolate isolate = Isolate.current;
    isolateReadyPort.send(_PlatformIsolateReadyMessage(
        isolate.controlPort,
        isolate.pauseCapability,
        isolate.terminateCapability,
        entryPointPort.sendPort));
  }

  @Native<Void Function(Handle, Handle, Handle)>(
      symbol: 'PlatformIsolateNativeApi::Spawn')
  external static void _nativeSpawn(Function entryPoint,
      SendPort isolateReadyPort, String debugName);

  static Future<SendPort>? _platformRunnerSendPort;

  /// Runs [computation] in an isolate on the platform thread and returns the
  /// result.
  ///
  /// This method can only be invoked from the main isolate.
  ///
  /// See [Isolate.run] for details.
  static FutureOr<R> run<R>(FutureOr<R> Function() computation) {
    if (_platformRunnerSendPort == null) {
      final Completer<SendPort> sendPortCompleter = Completer<SendPort>();
      _platformRunnerSendPort = sendPortCompleter.future;
      final RawReceivePort portReceiver = _receiveOne(
          (SendPort port) => sendPortCompleter.complete(port));
      // TODO(liamappelbe): If there are any in-flight computations when onExit
      // is triggered, complete them with errors.
      final RawReceivePort onExit = _receiveOne((_) => _platformRunnerSendPort = null);
      try {
        _spawn(_remoteRun, portReceiver.sendPort, onExit: onExit.sendPort);
      } on Object {
        portReceiver.close();
        onExit.close();
        rethrow;
      }
    }

    final Completer<R> resultCompleter = Completer<R>();
    final RawReceivePort resultPort = _receiveOne(((R? result, Object? remoteError, Object? remoteStack)? response) {
      if (response == null) {
        // onExit handler message, isolate terminated without sending result.
        resultCompleter.completeError(
            RemoteError('Computation ended without result', ''),
            StackTrace.empty);
        return;
      }
      final (R? result, Object? remoteError, Object? remoteStack) = response;
      if (remoteStack != null) {
        if (remoteStack is StackTrace) {
          // Typed error.
          resultCompleter.completeError(remoteError!, remoteStack);
        } else {
          // onError handler message, uncaught async error.
          // Both values are strings, so calling `toString` is efficient.
          final RemoteError error =
              RemoteError(remoteError!.toString(), remoteStack.toString());
          resultCompleter.completeError(error, error.stackTrace);
        }
      } else {
        resultCompleter.complete(result);
      }
    });

    _platformRunnerSendPort!.then(
        (port) => port.send((computation, resultPort.sendPort)));
    return resultCompleter.future;
  }

  static Future<void> _remoteRun<R>(SendPort portReceiver) async {
    final RawReceivePort computationPort = RawReceivePort();
    computationPort.handler =
        ((FutureOr<R> Function() computation, SendPort resultPort) message) async {
      // Once this isolate has handled at least one computation, allow it to
      // close if there are no more incoming.
      computationPort.keepIsolateAlive = false;

      final (FutureOr<R> Function() computation, SendPort resultPort) = message;
      try {
        final FutureOr<R> potentiallyAsyncResult = computation();
        late final R result;
        if (potentiallyAsyncResult is Future<R>) {
          result = await potentiallyAsyncResult;
        } else {
          result = potentiallyAsyncResult;
        }
        resultPort.send((result, null, null));
      } catch (e, s) {
        // If sending fails, the error becomes an uncaught error.
        resultPort.send((null, e, s));
      }
    };
    portReceiver.send(computationPort.sendPort);
  }

  /// Returns whether the current isolate is running on the platform thread.
  @Native<Bool Function()>(
      symbol: 'PlatformIsolateNativeApi::IsRunningOnPlatformThread')
  external static bool isRunningOnPlatformThread();

  static RawReceivePort _receiveOne<T>(void Function(T) then) {
    final RawReceivePort port = RawReceivePort();
    port.handler = (T value) {
      port.close();
      then(value);
    };
    return port;
  }
}

class _PlatformIsolateReadyMessage {
  _PlatformIsolateReadyMessage(this.controlPort, this.pauseCapability,
      this.terminateCapability, this.entryPointPort);

  final SendPort controlPort;
  final Capability? pauseCapability;
  final Capability? terminateCapability;
  final SendPort entryPointPort;
}
