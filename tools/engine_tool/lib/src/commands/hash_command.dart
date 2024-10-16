// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'package:convert/convert.dart';
import 'package:crypto/crypto.dart';
import 'command.dart';

/// Returns the hash signature of the engine sources used for downloading assets.
final class HashCommand extends CommandBase {
  /// Constructs the 'hash' command.
  HashCommand({
    required super.environment,
    super.usageLineLength,
  });

  @override
  String get name => 'hash';

  @override
  String get description => 'Generates the engine hash signature.';

  @override
  Future<int> run() async {
    var processResult = await environment.processRunner.processManager.run(
      <String>[
        'git',
        'merge-base',
        'upstream/main',
        'HEAD',
      ],
      workingDirectory: environment.engine.flutterDir.path,
    );

    if (processResult.exitCode != 0) {
      environment.logger.fatal('''
Error getting base hash of the repositor: ${processResult.exitCode}
${processResult.stderr}''');
    }

    final regex = RegExp(r'^([a-fA-F0-9]+)');
    final baseHash = regex.matchAsPrefix(processResult.stdout as String);
    if (baseHash == null || baseHash.groupCount != 1) {
      environment.logger.fatal('''
Error getting base hash of the repositor: ${processResult.exitCode}
${processResult.stdout}''');
    }

    // List the tree (not the working tree) recusrively for the merge-base.
    // This is importnat for future filtering of files, but also do not include
    // the developer's changes / in flight PRs.
    processResult = await environment.processRunner.processManager.run(
      <String>[
        'git',
        'ls-tree',
        '-r',
        baseHash[1]!,
        // Uncomment the following lines after merge
        // 'engine', 'DEPS'
      ],
      workingDirectory: environment.engine.flutterDir.path,
      stdoutEncoding: utf8,
    );

    if (processResult.exitCode != 0) {
      environment.logger.fatal('''
Error listing tree: ${processResult.exitCode}
${processResult.stderr}''');
    }

    // Ensure stable line endings so our hash calculation is stable
    final treeLines =
        LineSplitter.split(processResult.stdout as String);

    // We could call `git hash-object --stdin` which would just take the input, calculate the size,
    // and then sha1sum it like: `blob $size\0$string'. However, that deals with newlines.
    // Instead this is equivilant to:
    //     git ls-tree -r $(git merge-base upstream/main HEAD) | <only newlines> | sha1sum
    final output = AccumulatorSink<Digest>();
    final sink = sha1.startChunkedConversion(output);
    for (final line in treeLines) {
      sink.add(utf8.encode(line));
      sink.add([0x0a]);
    }
    sink.close();
    final digest = output.events.first;

    environment.stdout.write('$digest');
    return 0;
  }
}
