// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon;

/// A base class for classes that wrap Handles.
class _HandleWrapper<T> {
  Handle? _handle;

  _HandleWrapper(this._handle);

  Handle? get handle => _handle;
  bool get isValid => handle?.isValid ?? false;

  void close() {
    _handle!.close();
    _handle = null;
  }

  Handle? passHandle() {
    final Handle? result = _handle;
    _handle = null;
    return result;
  }

  @override
  bool operator ==(Object other) =>
      (other is _HandleWrapper) && handle == other.handle;

  @override
  int get hashCode => handle.hashCode;

  @override
  String toString() => '$runtimeType($handle)';
}

/// A base class for classes that wrap a pair of Handles.
abstract class _HandleWrapperPair<T> {
  final int status;
  final T first;
  final T second;
  _HandleWrapperPair._(this.status, this.first, this.second);
}
