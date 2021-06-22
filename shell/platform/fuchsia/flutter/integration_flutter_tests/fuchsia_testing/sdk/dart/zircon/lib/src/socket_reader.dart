// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon;

// ignore_for_file: public_member_api_docs

class SocketReaderError extends Error {
  final Object error;
  final StackTrace? stacktrace;

  SocketReaderError(this.error, this.stacktrace);

  @override
  String toString() => error.toString();
}

typedef SocketReaderReadableHandler = void Function();
typedef SocketReaderErrorHandler = void Function(SocketReaderError? error);

class SocketReader {
  Socket? get socket => _socket;
  Socket? _socket;

  bool get isBound => _socket != null;

  late HandleWaiter _waiter;

  SocketReaderReadableHandler? onReadable;
  SocketReaderErrorHandler? onError;

  void bind(Socket socket) {
    if (isBound) {
      throw ZirconApiError('SocketReader is already bound.');
    }
    _socket = socket;
    _asyncWait();
  }

  Socket? unbind() {
    if (!isBound) {
      throw ZirconApiError('SocketReader is not bound');
    }
    _waiter.cancel();
    final Socket? result = _socket;
    _socket = null;
    return result;
  }

  void close() {
    if (!isBound) {
      return;
    }
    _waiter.cancel();
    _socket!.close();
    _socket = null;
  }

  void _asyncWait() {
    _waiter = _socket!.handle!
        .asyncWait(Socket.READABLE | Socket.PEER_CLOSED, _handleWaitComplete);
  }

  void _errorSoon(SocketReaderError? error) {
    if (onError == null) {
      return;
    }
    scheduleMicrotask(() {
      // We need to re-check onError because it might have changed during the
      // asynchronous gap.
      if (onError != null) {
        onError!(error);
      }
    });
  }

  @override
  String toString() => 'SocketReader($_socket)';

  void _handleWaitComplete(int status, int pending) {
    assert(isBound);
    if (status != ZX.OK) {
      close();
      _errorSoon(SocketReaderError(
          'Wait completed with status ${getStringForStatus(status)} ($status)',
          null));
      return;
    }
    // TODO(abarth): Change this try/catch pattern now that we don't use
    // RawReceivePort any more.
    try {
      if ((pending & Socket.READABLE) != 0) {
        if (onReadable != null) {
          onReadable!();
        }
        if (isBound) {
          _asyncWait();
        }
      } else if ((pending & Socket.PEER_CLOSED) != 0) {
        close();
        _errorSoon(null);
      }
      // ignore: avoid_catching_errors
    } on Error catch (_) {
      // An Error exception from the core libraries is probably a programming
      // error that can't be handled. We rethrow the error so that
      // FidlEventHandlers can't swallow it by mistake.
      rethrow;
      // ignore: avoid_catches_without_on_clauses
    } catch (e, s) {
      close();
      _errorSoon(SocketReaderError(e, s));
    }
  }
}
