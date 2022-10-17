// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


import 'dart:convert';
import 'dart:io';
import 'package:http/http.dart' as http;
import 'package:meta/meta.dart' show immutable;
import 'package:path/path.dart' as path;

import 'data.dart';

import 'json_get.dart';
import 'layout_types.dart';

/// Signature for function that asynchonously returns a value.
typedef AsyncGetter<T> = Future<T> Function();

const String githubCacheFileName = 'github-response.json';
const String githubTargetFolder = 'src/vs/workbench/services/keybinding/browser/keyboardLayouts';

const String githubQuery = '''
{
  repository(owner: "microsoft", name: "vscode") {
    defaultBranchRef {
       target {
        ... on Commit {
          history(first: 1) {
            nodes {
              oid
              file(path: "$githubTargetFolder") {
                extension lineCount object {
                  ... on Tree {
                    entries {
                      name object {
                        ... on Blob {
                          text
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
''';

/// All goals in the form of KeyboardEvent.key.
final List<String> kGoalKeys = kLayoutGoals.keys.toList();

/// A map from the key of `kLayoutGoals` (KeyboardEvent.key) to an
/// auto-incremental index.
final Map<String, int> kGoalToIndex = Map<String, int>.fromEntries(
  kGoalKeys.asMap().entries.map(
    (MapEntry<int, String> entry) => MapEntry<String, int>(entry.value, entry.key)),
);

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

/// Retrieve a string using the procedure defined by `ifNotExist` based on the
/// cache file at `cachePath`.
///
/// If `forceRefresh` is false, this function tries to read the cache file, calls
/// `ifNotExist` when necessary, and writes the result to the cache.
///
/// If `forceRefresh` is true, this function never read the cache file, always
/// calls `ifNotExist` when necessary, and still writes the result to the cache.
///
/// Exceptions from `ifNotExist` will be thrown, while exceptions related to
/// caching are only printed.
Future<String> _tryCached(String cachePath, bool forceRefresh, AsyncGetter<String> ifNotExist) async {
  final File cacheFile = File(cachePath);
  if (!forceRefresh && cacheFile.existsSync()) {
    try {
      final String result = cacheFile.readAsStringSync();
      print('Using GitHub cache.');
      return result;
    } catch (exception) {
      print('Error reading GitHub cache, rebuilding. Details: $exception');
    }
  }
  final String result = await ifNotExist();
  IOSink? sink;
  try {
    print('Requesting from GitHub...');
    Directory(path.dirname(cachePath)).createSync(recursive: true);
    sink = cacheFile.openWrite();
    cacheFile.writeAsStringSync(result);
  } catch (exception) {
    print('Error writing GitHub cache. Details: $exception');
  } finally {
    sink?.close();
  }
  return result;
}

Future<Map<String, dynamic>> _fetchGithub(String githubToken, bool forceRefresh, String cachePath) async {
  final String response = await _tryCached(cachePath, forceRefresh, () async {
    final String condensedQuery = githubQuery
        .replaceAll(RegExp(r'\{ +'), '{')
        .replaceAll(RegExp(r' +\}'), '}');
    final http.Response response = await http.post(
      Uri.parse('https://api.github.com/graphql'),
      headers: <String, String>{
        'Content-Type': 'application/json; charset=UTF-8',
        'Authorization': 'bearer $githubToken',
      },
      body: jsonEncode(<String, String>{
        'query': condensedQuery,
      }),
    );
    if (response.statusCode != 200) {
      throw Exception('Request to GitHub failed with status code ${response.statusCode}: ${response.reasonPhrase}');
    }
    return response.body;
  });
  return jsonDecode(response) as Map<String, dynamic>;
}

@immutable
class _GitHubFile {
  const _GitHubFile({required this.name, required this.content});

  final String name;
  final String content;
}

_GitHubFile _jsonGetGithubFile(JsonContext<JsonArray> files, int index) {
  final JsonContext<JsonObject> file = jsonGetIndex<JsonObject>(files, index);
  return _GitHubFile(
    name: jsonGetKey<String>(file, 'name').current,
    content: jsonGetPath<String>(file, 'object.text').current,
  );
}

String _parsePrintable(String rawString) {
  // Parse a char represented in unicode hex, such as \u001b.
  final RegExp hexParser = RegExp(r'^\\u([0-9a-fA-F]+)$');

  if (rawString.isEmpty) {
    return '';
  }
  final RegExpMatch? hexMatch = hexParser.firstMatch(rawString);
  if (hexMatch != null) {
    final int codeUnit = int.parse(hexMatch.group(1)!, radix: 16);
    return String.fromCharCode(codeUnit);
  }
  return const <String, String>{
    r'\\': r'\',
    r'\r': '\r',
    r'\b': '\b',
    r'\t': '\t',
    r"\'": "'",
  }[rawString] ?? rawString;
}

Layout _parseLayoutFromGithubFile(_GitHubFile file) {
  final Map<String, LayoutEntry> entries = <String, LayoutEntry>{};

  // Parse a line that looks like the following, and get its key as well as
  // the content within the square bracket.
  //
  //    F19: [],
  //    KeyZ: ['y', 'Y', '', '', 0, 'VK_Y'],
  final RegExp lineParser = RegExp(r'^[ \t]*(.+?): \[(.*)\],$');
  // Parse each child of the content within the square bracket.
  final RegExp listParser = RegExp(r"^'(.*?)', '(.*?)', '(.*?)', '(.*?)', (\d)(?:, '(.+)')?$");
  file.content.split('\n').forEach((String line) {
    final RegExpMatch? lineMatch = lineParser.firstMatch(line);
    if (lineMatch == null) {
      return;
    }
    // KeyboardKey.key, such as "KeyZ".
    final String eventKey = lineMatch.group(1)!;
    // Only record goals.
    if (!kGoalToIndex.containsKey(eventKey)) {
      return;
    }

    // Comma-separated definition as a string, such as "'y', 'Y', '', '', 0, 'VK_Y'".
    final String definition = lineMatch.group(2)!;
    if (definition.isEmpty) {
      return;
    }
    // Group 1-4 are single strings for an entry, such as "y", "", "\u001b".
    // Group 5 is the dead mask.
    final RegExpMatch? listMatch = listParser.firstMatch(definition);
    assert(listMatch != null, 'Unable to match $definition');
    final int deadMask = int.parse(listMatch!.group(5)!, radix: 10);

    entries[eventKey] = LayoutEntry(
      <String>[
        _parsePrintable(listMatch.group(1)!),
        _parsePrintable(listMatch.group(2)!),
        _parsePrintable(listMatch.group(3)!),
        _parsePrintable(listMatch.group(4)!),
      ],
      deadMask,
    );
  });

  for (final String goalKey in kGoalKeys) {
    entries.putIfAbsent(goalKey, () => LayoutEntry.empty);
  }

  // Parse the file name, which looks like "en-belgian.win.ts".
  final RegExp fileNameParser = RegExp(r'^([^.]+)\.([^.]+)\.ts$');
  late final Layout layout;
  try {
    final RegExpMatch? match = fileNameParser.firstMatch(file.name);
    final String layoutName = match!.group(1)!;
    final LayoutPlatform platform = _platformFromGithubString(match.group(2)!);
    layout = Layout(layoutName, platform, entries);
  } catch (exception) {
    throw ArgumentError('Unrecognizable file name ${file.name}.');
  }
  return layout;
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


LayoutPlatform _platformFromGithubString(String origin) {
  switch (origin) {
    case 'win':
      return LayoutPlatform.win;
    case 'linux':
      return LayoutPlatform.linux;
    case 'darwin':
      return LayoutPlatform.darwin;
    default:
      throw ArgumentError('Unexpected platform "$origin".');
  }
}

int _sortLayout(Layout a, Layout b) {
  int result = a.language.compareTo(b.language);
  if (result == 0) {
    result = a.platform.index.compareTo(b.platform.index);
  }
  return result;
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
  return lines.sublist(startLine + 1, endLine).join('\n');
}

Future<void> generate(Options options) async {
  // Fetch files from GitHub.
  final Map<String, dynamic> githubBody = await _fetchGithub(
    options.githubToken,
    options.force,
    path.join(options.cacheRoot, githubCacheFileName),
  );

  // Parse the result from GitHub.
  final JsonContext<JsonObject> commitJson = jsonGetPath<JsonObject>(
    JsonContext.root(githubBody),
    'data.repository.defaultBranchRef.target.history.nodes.0',
  );
  final String commitId = jsonGetKey<String>(commitJson, 'oid').current;
  final JsonContext<JsonArray> fileListJson = jsonGetPath<JsonArray>(
    commitJson,
    'file.object.entries',
  );
  final Iterable<_GitHubFile> files = Iterable<_GitHubFile>.generate(
    fileListJson.current.length,
    (int index) => _jsonGetGithubFile(fileListJson, index),
  ).where(
    // Exclude controlling files, which contain no layout information.
    (_GitHubFile file) => !file.name.startsWith('layout.contribution.')
                      && !file.name.startsWith('_.contribution'),
  );

  // Layouts must be sorted to ensure that the output file has a fixed order.
  final List<Layout> layouts = files.map(_parseLayoutFromGithubFile)
    .toList()
    ..sort(_sortLayout);

  // Build store.
  final LayoutStore store = LayoutStore(kLayoutGoals, layouts);
  final String body = marshallStoreCompressed(store);

  // Verify that the store can be unmarshalled correctly.
  // Inconcistencies will cause exceptions.
  verifyLayoutStoreEqual(store, unmarshallStoreCompressed(body));

  // Generate the definition file.
  _writeFileTo(
    options.outputRoot,
    'definitions.g.dart',
    _renderTemplate(
      File(path.join(options.dataRoot, 'definitions.dart.tmpl')).readAsStringSync(),
      <String, String>{
        'COMMIT_ID': commitId,
        'BODY': _prettyPrintBody(body, 64),
        'BODY_LENGTH': '${body.length}',
      },
    ),
  );

  // Generate the type file.
  _writeFileTo(
    options.outputRoot,
    'types.g.dart',
    _renderTemplate(
      File(path.join(options.dataRoot, 'types.dart.tmpl')).readAsStringSync(),
      <String, String>{
        'BODY': _readSharedSegment(path.join(options.libRoot, 'layout_types.dart')),
      },
    ),
  );

  // Generate the JSON file.
  _writeFileTo(
    options.dataRoot,
    'definitions_uncompressed.g.json',
    const JsonEncoder.withIndent('  ').convert(jsonifyStore(store)),
  );
}
