// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io';

import 'package:convert/convert.dart';
import 'package:crypto/crypto.dart';

/// Returns the hash signature for the engine source code.
///
/// By default, [useMergeBase] will find the base commit hash for local development.
/// To generate a hash for the local HEAD, set useMergeBase to false, e.g. when building
/// a release from a branch with cherry picks.
Future<({String result, String? error})> engineHash(
  Future<ProcessResult> Function(List<String> command) runProccess, {
  bool useMergeBase = true,
}) async {
  final String base;

  if (useMergeBase) {
    final processResult = await runProccess(
      <String>[
        'git',
        'merge-base',
        'upstream/main',
        'HEAD',
      ],
    );

    if (processResult.exitCode != 0) {
      return (
        result: '',
        error: '''
  Error getting base hash of the repositor: ${processResult.exitCode}
  ${processResult.stderr}''',
      );
    }

    final regex = RegExp(r'^([a-fA-F0-9]+)');
    final baseHash = regex.matchAsPrefix(processResult.stdout as String);
    if (baseHash == null || baseHash.groupCount != 1) {
      return (
        result: '',
        error: '''
  Error getting base hash of the repositor: ${processResult.exitCode}
  ${processResult.stdout}''',
      );
    }
    base = baseHash[1]!;
  } else {
    base = 'HEAD';
  }

  // List the tree (not the working tree) recusrively for the merge-base.
  // This is importnat for future filtering of files, but also do not include
  // the developer's changes / in flight PRs.
  final processResult = await runProccess(
    <String>[
      'git',
      'ls-tree',
      '-r',
      base,
      // Uncomment the following lines after merge
      // 'engine', 'DEPS'
    ],
  );

  if (processResult.exitCode != 0) {
    return (
      result: '',
      error: '''
Error listing tree: ${processResult.exitCode}
${processResult.stderr}''',
    );
  }

  // Ensure stable line endings so our hash calculation is stable
  final treeLines = LineSplitter.split(processResult.stdout as String);

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

  return (result: '$digest', error: null);
}
