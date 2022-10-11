// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:gen_web_keyboard_layouts/data.dart';
import 'package:meta/meta.dart' show immutable;

import 'dart:convert';
import 'dart:io';
import 'package:http/http.dart' as http;
import 'package:path/path.dart' as path;

import 'json_get.dart';
import 'utils.dart';

const int kDeadChar = 0x1000000;

@immutable
class Options {
  /// Build an option.
  const Options({
    required this.force,
    required this.githubToken,
    required this.cacheRoot,
    required this.dataRoot,
    required this.outputRoot,
  });

  final bool force;

  /// The GitHub personal access token used to make the GitHub request.
  final String githubToken;

  /// The path of the folder that store cache.
  final String cacheRoot;

  /// The path of the folder that store data files, such as templates.
  final String dataRoot;

  /// The folder to store the output Dart files.
  final String outputRoot;
}

const String githubCacheFileName = 'github-response.json';
const String githubTargetFolder = 'src/vs/workbench/services/keybinding/browser/keyboardLayouts';
const String overallTemplateName = 'layouts.dart.tmpl';
const String entryTemplateName = 'layout_entry.dart.tmpl';
const String outputName = 'definitions.g.dart';

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

/// Signature for function that asynchonously returns a value.
typedef AsyncGetter<T> = Future<T> Function();

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
Future<String> tryCached(String cachePath, bool forceRefresh, AsyncGetter<String> ifNotExist) async {
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

Future<Map<String, dynamic>> fetchGithub(String githubToken, bool forceRefresh, String cachePath) async {
  final String response = await tryCached(cachePath, forceRefresh, () async {
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
class GitHubFile {
  const GitHubFile({required this.name, required this.content});

  final String name;
  final String content;
}

GitHubFile jsonGetGithubFile(JsonContext<JsonArray> files, int index) {
  final JsonContext<JsonObject> file = jsonGetIndex<JsonObject>(files, index);
  return GitHubFile(
    name: jsonGetKey<String>(file, 'name').current,
    content: jsonGetPath<String>(file, <String>['object', 'text']).current,
  );
}

typedef LayoutEntry = List<int>;

@immutable
class Layout {
  const Layout(this.name, this.platform, this.mapping);

  final String name;
  final String platform;
  final Map<String, LayoutEntry> mapping;
}

Layout parseLayoutFile(GitHubFile file) {
  final Map<String, LayoutEntry> mapping = <String, LayoutEntry>{};

  // Parse a line that looks like the following, and get its key as well as
  // the content within the square bracket.
  //
  //    F19: [],
  //    KeyZ: ['y', 'Y', '', '', 0, 'VK_Y'],
  final RegExp lineParser = RegExp(r'^[ \t]*(.+?): \[(.*)\],$');
  // Parse each child of the content within the square bracket.
  final RegExp listParser = RegExp(r"^'(.*?)', '(.*?)', '(.*?)', '(.*?)', (\d)(?:, '(.+)')?$");
  // Parse a char represented in unicode hex, such as \u001b.
  final RegExp hexParser = RegExp(r'^\\u([0-9a-fA-F]+)$');
  file.content.split('\n').forEach((String line) {
    final RegExpMatch? lineMatch = lineParser.firstMatch(line);
    if (lineMatch == null) {
      return;
    }
    final String eventKey = lineMatch.group(1)!;
    final String definition = lineMatch.group(2)!;
    if (definition.isEmpty) {
      return;
    }
    final RegExpMatch? listMatch = listParser.firstMatch(definition);
    assert(listMatch != null, 'Unable to match $definition');
    final int deadMask = int.parse(listMatch!.group(5)!, radix: 10);

    int combineValue(String rawString, int deadMask) {
      if (deadMask != 0 || rawString.isEmpty) {
        return kDeadChar;
      }
      final RegExpMatch? hexMatch = hexParser.firstMatch(rawString);
      if (hexMatch != null) {
        return int.parse(hexMatch.group(1)!, radix: 16);
      }
      final String charString = const <String, String>{
        r'\\': r'\',
        r'\r': '\r',
        r'\b': '\b',
        r'\t': '\t',
        r"\'": "'",
        'l̥': 'l', // TODO
        'L̥': 'L', // TODO
        'r̥': 'r', // TODO
        'R̥': 'R', // TODO
      }[rawString] ?? rawString;
      assert(charString.length == 1, 'Unrecognized multibyte character |$charString| (file ${file.name} key $eventKey)');
      return charString.codeUnitAt(0);
    }
    mapping[eventKey] = <int>[
      combineValue(listMatch.group(1)!, deadMask & 0x1),
      combineValue(listMatch.group(2)!, deadMask & 0x2),
      combineValue(listMatch.group(3)!, deadMask & 0x4),
      combineValue(listMatch.group(4)!, deadMask & 0x8),
    ];
  });

  // Parse the file name, which looks like "en-belgian.win.ts".
  final RegExp fileNameParser = RegExp(r'^([^.]+)\.([^.]+)\.ts$');
  Layout? layout;
  try {
    final RegExpMatch? match = fileNameParser.firstMatch(file.name);
    final String layoutName = match!.group(1)!;
    final String platform = match.group(2)!;
    layout = Layout(layoutName, platform, mapping);
  } catch (exception) {
    throw ArgumentError('Unrecognizable file name ${file.name}.');
  }
  return layout;
}

String renderTemplate(String template, Map<String, String> dictionary) {
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

Future<void> generate(Options options) async {
  // Fetch files from GitHub.
  final Map<String, dynamic> githubBody = await fetchGithub(
    options.githubToken,
    options.force,
    path.join(options.cacheRoot, githubCacheFileName),
  );

  // Parse the result from GitHub.
  final JsonContext<JsonObject> commitJson = jsonGetPath<JsonObject>(
    JsonContext.root(githubBody),
    jsonPathSplit('data.repository.defaultBranchRef.target.history.nodes.0'),
  );
  final String commitId = jsonGetKey<String>(commitJson, 'oid').current;
  final JsonContext<JsonArray> fileListJson = jsonGetPath<JsonArray>(
    commitJson,
    jsonPathSplit('file.object.entries'),
  );
  final Iterable<GitHubFile> files = Iterable<GitHubFile>.generate(
    fileListJson.current.length,
    (int index) => jsonGetGithubFile(fileListJson, index),
  ).where(
    // A few files in the folder are controlling files, containing no layout
    // information.
    (GitHubFile file) => !file.name.startsWith('layout.contribution.')
                      && !file.name.startsWith('_.contribution'),
  );

  final List<Layout> layouts = files.map(parseLayoutFile).toList();

  final Iterable<String> entriesString = layouts.map((Layout layout) {
    return renderTemplate(
      File(path.join(options.dataRoot, entryTemplateName)).readAsStringSync(),
      <String, String>{
        'NAME': layout.name,
        'PLATFORM': layout.platform,
        'ENTRIES': kLayoutGoals.keys.map((String key) {
          final String value = layout.mapping[key]!.map(toHex).join(', ');
          return '      <int>[$value], // $key';
        }).join('\n'),
      },
    ).trimRight();
  });
  final String result = renderTemplate(
    File(path.join(options.dataRoot, overallTemplateName)).readAsStringSync(),
    <String, String>{
      'COMMIT_ID': commitId,
      'LAYOUT_GOALS': kLayoutGoals.entries.map((MapEntry<String, String?> entry) =>
        "  '${entry.key}': ${entry.value == null ? 'null' : "'${entry.value}'"},"
      ).join('\n'),
      'LAYOUT_ENTRIES': entriesString.join('\n\n'),
    },
  );
  final String outputPath = path.join(options.outputRoot, outputName);
  Directory(path.dirname(outputPath)).createSync(recursive: true);
  File(outputPath).writeAsStringSync(result);
}
