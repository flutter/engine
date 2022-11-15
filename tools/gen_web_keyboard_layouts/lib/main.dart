// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


import 'dart:io';
import 'package:meta/meta.dart' show immutable;
import 'package:path/path.dart' as path;

import 'benchmark_detector.dart';
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

typedef _ForEachAction<V> = void Function(String key, V value);
void _sortedForEach<V>(Map<String, V> map, _ForEachAction<V> action) {
  map
    .entries
    .toList()
    ..sort((MapEntry<String, V> a, MapEntry<String, V> b) => a.key.compareTo(b.key))
    ..forEach((MapEntry<String, V> entry) {
      action(entry.key, entry.value);
    });
}

String _escapeEventKey(String original) {
  switch (original) {
    case "'":
      return '"\'"';
    case r'\':
      return r"r'\'";
    case r'$':
      return r"r'$'";
    default:
      return "'$original'";
  }
}

String _buildMapString(Iterable<Layout> layouts) {
  final List<String> codeStrings = <String>[];
  _sortedForEach(buildMap(layouts), (String eventCode, Map<String, int> eventKeyToLogicalKeys) {
    final List<String> codeStringBodies = <String>[];
    _sortedForEach(eventKeyToLogicalKeys, (String eventKey, int result) {
      codeStringBodies.add('    ${_escapeEventKey(eventKey)}: 0x${result.toRadixString(16)},');
    });
    codeStrings.add('''
  '$eventCode': <String, int>{
${codeStringBodies.join('\n').trimRight()}
  },''');
  });
  return '<String, Map<String, int>>{\n${codeStrings.join('\n')}\n}';
}

String _buildTestCasesString(List<Layout> layouts) {
  final List<String> layoutsString = <String>[];
  for (final Layout layout in layouts) {
    final List<String> layoutEntries = <String>[];
    _sortedForEach(buildLayout(layout.entries), (String eventCode, int logicalKey) {
      final LayoutEntry entry = layout.entries[eventCode]!;
      layoutEntries.add("    verifyEntry(mapping, '$eventCode', <String>["
          '${entry.printables.map(_escapeEventKey).join(', ')}'
          "], '${String.fromCharCode(logicalKey)}');");
    });
    layoutsString.add('''
  group('${layout.language}', () {
${layoutEntries.join('\n')}
  });
''');
  }
  return layoutsString.join('\n').trimRight();
}

Future<void> generate(Options options) async {
  final GithubResult githubResult = await fetchFromGithub(
    githubToken: options.githubToken,
    force: options.force,
    cacheRoot: options.cacheRoot,
  );
  // Build store.
  final List<Layout> winLayouts = githubResult.layouts.where((Layout layout) =>
            layout.platform == LayoutPlatform.win).toList();
  final List<Layout> linuxLayouts = githubResult.layouts.where((Layout layout) =>
            layout.platform == LayoutPlatform.linux).toList();
  final List<Layout> darwinLayouts = githubResult.layouts.where((Layout layout) =>
            layout.platform == LayoutPlatform.darwin).toList();

  // Generate the definition file.
  _writeFileTo(
    path.join(options.outputRoot, 'lib', 'web_keyboard_layouts'),
    'key_mappings.g.dart',
    _renderTemplate(
      File(path.join(options.dataRoot, 'key_mappings.dart.tmpl')).readAsStringSync(),
      <String, String>{
        'COMMIT_URL': githubResult.url,
        'WIN_MAPPING': _buildMapString(winLayouts),
        'LINUX_MAPPING': _buildMapString(linuxLayouts),
        'DARWIN_MAPPING': _buildMapString(darwinLayouts),
        'COMMON': _readSharedSegment(path.join(options.libRoot, 'common.dart')),
      },
    ),
  );

  // Generate the test cases.
  _writeFileTo(
    path.join(options.outputRoot, 'test'),
    'test_cases.g.dart',
    _renderTemplate(
      File(path.join(options.dataRoot, 'test_cases.dart.tmpl')).readAsStringSync(),
      <String, String>{
        'WIN_CASES': _buildTestCasesString(winLayouts),
        'LINUX_CASES': _buildTestCasesString(linuxLayouts),
        'DARWIN_CASES': _buildTestCasesString(darwinLayouts),
      },
    ),
  );
}
