// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'package:path/path.dart' as p;
import 'package:process_runner/process_runner.dart';

import 'environment.dart';
import 'proc_utils.dart';

/// Information about a test build target.
final class TestTarget {
  /// Construct a test target.
  TestTarget(this.label, this.executable);

  /// The build target label.
  final String label;

  /// The executable file produced after the build target is built.
  final File executable;

  @override
  String toString() {
    return 'target=$label executable=${executable.path}';
  }
}

/// Returns test targets for a given build directory.
Future<Map<String, TestTarget>> findTestTargets(
    Environment environment, Directory buildDir) async {
  final Map<String, TestTarget> r = <String, TestTarget>{};
  final List<String> getLabelsCommandLine = <String>[
    'gn',
    'ls',
    buildDir.path,
    '--type=executable',
    '--testonly=true',
    '--as=label',
  ];
  final ProcessRunnerResult labelsResult = await environment.processRunner
      .runProcess(getLabelsCommandLine,
          workingDirectory: environment.engine.srcDir, failOk: true);
  fatalIfFailed(environment, getLabelsCommandLine, labelsResult);
  final String rawLabels = labelsResult.stdout;
  final List<String> getOutputsCommandLine = <String>[
    'gn',
    'ls',
    buildDir.path,
    '--type=executable',
    '--testonly=true',
    '--as=output'
  ];
  final ProcessRunnerResult outputsResults = await environment.processRunner
      .runProcess(getOutputsCommandLine,
          workingDirectory: environment.engine.srcDir, failOk: true);
  fatalIfFailed(environment, getOutputsCommandLine, outputsResults);
  final String rawOutputs = outputsResults.stdout;
  final List<String> labels = rawLabels.split('\n');
  final List<String> outputs = rawOutputs.split('\n');
  if (labels.length != outputs.length) {
    environment.logger.fatal(
        'gn ls output is inconsistent A and B should be the same length:\nA=$labels\nB=$outputs');
  }
  // Drop the empty line at the end of the output.
  if (labels.isNotEmpty) {
    assert(labels.last.isEmpty);
    assert(outputs.last.isEmpty);
    labels.removeLast();
    outputs.removeLast();
  }
  for (int i = 0; i < labels.length; i++) {
    final String label = labels[i];
    final String output = outputs[i];
    assert(label.isNotEmpty);
    assert(output.isNotEmpty);
    r[label] = TestTarget(label, File(p.join(buildDir.path, output)));
  }
  return r;
}

/// Process selectors and filter allTargets for matches.
///
/// We support:
///   1) Exact label matches (the '//' prefix will be stripped off).
///   2) '/...' suffix which selects all targets that match the prefix.
Set<TestTarget> selectTargets(
    List<String> selectors, Map<String, TestTarget> allTargets) {
  final Set<TestTarget> selected = <TestTarget>{};
  for (String selector in selectors) {
    if (!selector.startsWith('//')) {
      // Insert the // prefix when necessary.
      selector = '//$selector';
    }
    const String recursiveSuffix = '/...';
    final bool prefixMatch = selector.endsWith(recursiveSuffix);
    if (prefixMatch) {
      // Remove the /... suffix.
      selector =
          selector.substring(0, selector.length - recursiveSuffix.length);
      // TODO(johnmccutchan): Accelerate this by using a trie.
      for (final TestTarget target in allTargets.values) {
        if (target.label.startsWith(selector)) {
          selected.add(target);
        }
      }
    } else {
      for (final TestTarget target in allTargets.values) {
        if (target.label == selector) {
          selected.add(target);
        }
      }
    }
  }
  return selected;
}
