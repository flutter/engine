// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:developer';

import 'error.dart';

// ignore_for_file: avoid_catches_without_on_clauses

// Common functions used by fidl bindings

/// Calls close and prints the exception.
void handleException(String name, dynamic exception, void Function() close) {
  close();
  print('Exception handling method call $name: $exception');
}

/// Wraps work with common try/catch behaviour and timeline events.
void performWithExceptionHandling(
    String name, void Function() work, void Function() close) {
  try {
    Timeline.startSync(name);
    work();
  } catch (_e) {
    handleException(name, _e, close);
    rethrow;
  } finally {
    Timeline.finishSync();
  }
}

/// Calls close and signals the error on the ctrl.
void handleCtrlError(dynamic ctrl, String message) {
  ctrl.proxyError(FidlError(message));
  ctrl.close();
}

/// Wraps work with common try/catch behaviour and timeline events.
void performCtrlWithExceptionHandling(
    String name, dynamic ctrl, void Function() work, String type) {
  try {
    Timeline.startSync(name);
    work();
  } catch (_e) {
    handleCtrlError(ctrl, 'Exception handling $type $name: $_e');
    rethrow;
  } finally {
    Timeline.finishSync();
  }
}
