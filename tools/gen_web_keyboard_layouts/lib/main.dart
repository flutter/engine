// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


import 'dart:io';
import 'package:meta/meta.dart' show immutable;
import 'package:path/path.dart' as path;

import 'benchmark_detector.dart';
import 'data.dart';
import 'github.dart';
import 'layout_types.dart';

@immutable
class Options {
  /// Build an option.
  const Options({
    required this.force,
    required this.githubToken,
    required this.cacheRoot,
    required this.dataRoot,
    required this.libRoot,
    required this.outputRoot,
  });

  final bool force;

  /// The GitHub personal access token used to make the GitHub request.
  final String githubToken;

  /// The path of the folder that store cache.
  final String cacheRoot;

  /// The path of the folder that store data files, such as templates.
  final String dataRoot;

  /// The path of the folder that store input lib files, typically the folder
  /// that contains this file.
  final String libRoot;

  /// The folder to store the output Dart files.
  final String outputRoot;
}

String _renderTemplate(
  String template, Map<String, String> dictionary) {
  String result = template;
  dictionary.forEach((String key, String value) {
    final String localResult = result.replaceAll('@@@$key@@@', value);
    if (localResult == result) {
      print('Template key $key is not used.');
    }
    result = localResult;
  });
  return result;
}

void _writeFileTo(
    String outputDir,
    String outputFileName,
    String body) {
  final String outputPath = path.join(outputDir, outputFileName);
  Directory(outputDir).createSync(recursive: true);
  File(outputPath).writeAsStringSync(body);
}


String _prettyPrintBody(String body, int width) {
  int min(int a, int b)  {
    return a < b ? a : b;
  }
  final List<String> result = <String>[];
  int start = 0;
  while (start < body.length) {
    final String row = body.substring(start, min(body.length, start + width));
    result.add("  '$row'");
    start += width;
  }
  return result.join('\n');
}

String _readSharedSegment(String path) {
  const String kSegmentStartMark = '/*@@@ SHARED SEGMENT START @@@*/';
  const String kSegmentEndMark = '/*@@@ SHARED SEGMENT END @@@*/';
  final List<String> lines = File(path).readAsStringSync().split('\n');
  // Defining the two variables as `late final` ensures that each mark is found
  // once and only once, otherwise assertion errors will be thrown.
  late final int startLine;
  late final int endLine;
  for (int lineNo = 0; lineNo < lines.length; lineNo += 1) {
    if (lines[lineNo] == kSegmentStartMark) {
      startLine = lineNo;
    } else if (lines[lineNo] == kSegmentEndMark) {
      endLine = lineNo;
    }
  }
  assert(startLine < endLine);
  return lines.sublist(startLine + 1, endLine).join('\n').trimRight();
}

Future<void> generate(Options options) async {
  final List<Layout> layouts = await fetchFromGithub(
    githubToken: options.githubToken,
    force: options.force,
    cacheRoot: options.cacheRoot,
  );
  // Build store.
  final LayoutStore store = LayoutStore(kLayoutGoals, layouts);
  buildMap(store.layouts.where((Layout layout) => layout.platform == LayoutPlatform.win));
  buildMap(store.layouts.where((Layout layout) => layout.platform == LayoutPlatform.linux));
  buildMap(store.layouts.where((Layout layout) => layout.platform == LayoutPlatform.darwin));

  // final String body = marshallStoreCompressed(store);

  // // Verify that the store can be unmarshalled correctly.
  // // Inconcistencies will cause exceptions.
  // verifyLayoutStoreEqual(store, unmarshallStoreCompressed(body));

  // // Generate the definition file.
  // _writeFileTo(
  //   options.outputRoot,
  //   'definitions.g.dart',
  //   _renderTemplate(
  //     File(path.join(options.dataRoot, 'definitions.dart.tmpl')).readAsStringSync(),
  //     <String, String>{
  //       'COMMIT_ID': commitId,
  //       'BODY': _prettyPrintBody(body, 64),
  //       'BODY_LENGTH': '${body.length}',
  //     },
  //   ),
  // );

  // // Generate the type file.
  // _writeFileTo(
  //   options.outputRoot,
  //   'types.g.dart',
  //   _renderTemplate(
  //     File(path.join(options.dataRoot, 'types.dart.tmpl')).readAsStringSync(),
  //     <String, String>{
  //       'BODY': _readSharedSegment(path.join(options.libRoot, 'layout_types.dart')),
  //     },
  //   ),
  // );

  // // Generate the JSON file.
  // _writeFileTo(
  //   options.dataRoot,
  //   'definitions_uncompressed.g.json',
  //   const JsonEncoder.withIndent('  ').convert(jsonifyStore(store)),
  // );
}
