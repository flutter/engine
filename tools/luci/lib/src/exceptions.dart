// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6

/// An exception used to signal that a command-line tool must exit.
///
/// The tool is expected to print the [message] and exit with the provided
/// [exitCode]. By default the tool should not print the stack trace from
/// this exception. The [message] is expected to contain all the information
/// relevant to the user, including stack traces, if any.
class ToolException implements Exception {
  /// Creates an exception signalling the LUCI tool to exit.
  ///
  /// Optionally [exitCode] can be provided (defaults to 1).
  ToolException(this.message, { this.exitCode = 1 });

  /// The error message.
  final String message;

  /// The exit code.
  final int exitCode;

  @override
  String toString() => '$ToolException($exitCode): $message';
}
