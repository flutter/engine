// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'package:args/args.dart';
import 'package:gen_web_keyboard_layouts/main.dart';
import 'package:path/path.dart' as path;

const String kEnvGithubToken = 'GITHUB_TOKEN';

Future<void> main(List<String> rawArguments) async {
  final Map<String, String> env = Platform.environment;
  final ArgParser argParser = ArgParser();
  argParser.addFlag(
    'force',
    abbr: 'f',
    negatable: false,
    help: 'Make a new request to GitHub even if a cache is detected',
  );
  argParser.addFlag(
    'help',
    abbr: 'h',
    negatable: false,
    help: 'Print help for this command.',
  );

  final ArgResults parsedArguments = argParser.parse(rawArguments);

  if (parsedArguments['help'] as bool) {
    print(argParser.usage);
    exit(0);
  }

  bool enabledAssert = false;
  assert(() {
    enabledAssert = true;
    return true;
  }());
  if (!enabledAssert) {
    print('Error: This script must be run with assert enabled. Please rerun with --enable-asserts.');
    exit(1);
  }

  final String? envGithubToken = env[kEnvGithubToken];
  if (envGithubToken == null) {
    print('Error: Environment variable GITHUB_TOKEN not found.\n\n'
          'Set the environment variable GITHUB_TOKEN as a GitHub personal access\n'
          'token for authentication. This token is only used for quota controlling\n'
          'and does not need any scopes. Create one at\n'
          'https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token.',
    );
    exit(1);
  }

  // The root of this package. The folder that is called
  // 'gen_web_keyboard_layouts' and contains 'pubspec.yaml'.
  final Directory packageRoot = Directory(path.dirname(Platform.script.toFilePath())).parent;

  await generate(Options(
    githubToken: envGithubToken,
    cacheRoot: path.join(packageRoot.path, '.cache'),
    dataRoot: path.join(packageRoot.path, 'data'),
    libRoot: path.join(packageRoot.path, 'lib'),
    force: parsedArguments['force'] as bool,
    outputRoot: path.join(packageRoot.parent.parent.path,
        'third_party', 'web_keyboard_layouts'),
  ));
}
