// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(fxb/68629): Remove the ignore tag.
//ignore_for_file: import_of_legacy_library_into_null_safe

import 'dart:async';
import 'dart:io';
import 'dart:isolate';

import 'package:logging/logging.dart';
import 'package:meta/meta.dart';

import '_log_message.dart';

const int _maxGlobalTags = 4; // leave one slot for code location
const int _maxTagLength = 63;

final _fileNameRegex = RegExp(r'(\w+\.dart)');
final _lineNumberRegex = RegExp(r'\.dart:(\d+)');

/// The base class for which log writers will inherit from. This class is
/// used to pipe logs from the onRecord stream
abstract class LogWriter {
  List<String> _globalTags = [];

  StreamController<LogMessage>? _controller;

  /// A name which will prefix each log message and can be used for filtering.
  String? loggerBaseName;

  /// If set to true, this method will include the stack trace
  /// in each log record so we can later extract out the call site.
  /// This is a heavy operation and should be used with caution.
  bool forceShowCodeLocation = false;

  /// Constructor
  LogWriter({
    required Stream<LogRecord> logStream,
    bool shouldBufferLogs = false,
  }) {
    void Function(LogMessage) onMessageFunc;

    if (shouldBufferLogs) {
      // create single subscription stream controller so that we buffer calls to the
      // stream while we connect to the logger. This avoids dropping logs that
      // come in while we wait.
      _controller = StreamController<LogMessage>();

      onMessageFunc = _controller!.add;
    } else {
      onMessageFunc = onMessage;
    }
    logStream.listen((record) => onMessageFunc(_messageFromRecord(record)),
        onDone: () => _controller?.close());
  }

  /// The global tags to add to each log record.
  set globalTags(List<String?> tags) => _globalTags = _verifyGlobalTags(tags);

  /// Remaps the level string to the ones used in FTL.
  String getLevelString(Level level) {
    if (level == Level.FINE) {
      return 'VLOG(1)';
    } else if (level == Level.FINER) {
      return 'VLOG(2)';
    } else if (level == Level.FINEST) {
      return 'VLOG(3)';
    } else if (level == Level.SEVERE) {
      return 'ERROR';
    } else if (level == Level.SHOUT) {
      return 'FATAL';
    } else {
      return level.toString();
    }
  }

  LogMessage _messageFromRecord(LogRecord record) => LogMessage(
        record: record,
        processId: pid,
        threadId: Isolate.current.hashCode,
        loggerBaseName: loggerBaseName,
        codeLocation: _codeLocation(),
        tags: _globalTags,
      );

  /// A method for subclasses to implement to handle messages as they are
  /// written
  void onMessage(LogMessage message);

  /// A method which is exposed to subclasses which can be used to indicate that
  /// they are ready to start receiving messages.
  @protected
  void startListening(void Function(LogMessage)? onMessage) =>
      _controller!.stream.listen(onMessage);

  List<String> _verifyGlobalTags(List<String?> tags) {
    List<String> result = <String>[];

    // make our own copy to allow us to remove null values and not change the
    // original values
    final incomingTags = List.of(tags)
      ..removeWhere((t) => t == null || t.isEmpty);

    if (incomingTags.length > _maxGlobalTags) {
      Logger.root.warning('Logger initialized with > $_maxGlobalTags tags.');
      Logger.root.warning('Later tags will be ignored.');
    }
    for (int i = 0; i < _maxGlobalTags && i < incomingTags.length; i++) {
      String s = incomingTags[i]!;
      if (s.length > _maxTagLength) {
        Logger.root
            .warning('Logger tags limited to $_maxTagLength characters.');
        Logger.root.warning('Tag "$s" will be truncated.');
        s = s.substring(0, _maxTagLength);
      }
      result.add(s);
    }

    return result;
  }

  String? _codeLocation() {
    if (forceShowCodeLocation) {
      return _codeLocationFromStackTrace(StackTrace.current);
    }
    return null;
  }

  String? _codeLocationFromStackTrace(StackTrace stackTrace) {
    final lines = stackTrace.toString().split('\n');
    // There is no well supported way for getting the calling code location from
    // the log line. In lieu of this, we use the following algorithm which is
    // fragile and depends on the order which the logger methods are called. The
    // algorithm is that we run through each line and look for the call to the
    // line that contains 'Logger.log (package:logging/logging.dart' which is
    // the call that comes after the user calls Logger.info, Logger.warn, etc.
    // When this line is found we look 2 lines past for their call site.

    // Newer versions of the logging package use logging/src/logger.dart.
    final loggerLogLine = RegExp(r'Logger\.log \(package:logging/logging\.dart'
        r'|Logger\.log \(package:logging/src/logger\.dart');
    const logLineOffset = 2;
    String? codeLocation;

    for (int lineNumber = 0; lineNumber < lines.length; lineNumber++) {
      final line = lines[lineNumber];
      if (line.contains(loggerLogLine)) {
        if (lineNumber + logLineOffset < lines.length) {
          codeLocation = lines[lineNumber + logLineOffset];
        }
        break;
      }
    }

    return _extractCodeLocationFromLine(codeLocation);
  }

  String? _extractCodeLocationFromLine(String? line) {
    if (line == null) {
      return null;
    }

    String? regexValue(RegExp regexp) {
      final match = regexp.firstMatch(line);
      return match != null ? match.group(1) : null;
    }

    final fileName = regexValue(_fileNameRegex);
    if (fileName == null) {
      return null;
    }

    final lineNumber = regexValue(_lineNumberRegex);

    // Some environments don't give us the line number so we avoid failing
    // completely if we have just the file name.
    return lineNumber == null ? fileName : '$fileName($lineNumber)';
  }
}
