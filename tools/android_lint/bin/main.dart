// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'package:args/args.dart';
import 'package:path/path.dart' as path;
import 'package:process/process.dart';

Future<void> main(List<String> args) async {
  final ArgParser argParser = ArgParser();
  argParser.addOption(
    'in',
    help: 'The path to `engine/src`.',
    defaultsTo: path.relative(
      path.join(
        path.dirname(
          path.dirname(path.dirname(path.fromUri(Platform.script))),
        ),
        '..',
        '..',
      ),
    ),
  );
  argParser.addOption(
    'out',
    help: 'The path to write the generated the HTML report to.',
    defaultsTo: 'lint_report',
  );
  argParser.addFlag(
    'help',
    help: 'Print usage of the command.',
    negatable: false,
    defaultsTo: false,
  );

  final ArgResults argResults = argParser.parse(args);
  if (argResults['help']) {
    print(argParser.usage);
    exit(0);
  }
  final Directory androidDir = Directory(path.join(
    argResults['in'],
    'flutter',
    'shell',
    'platform',
    'android',
  ));
  if (!androidDir.existsSync()) {
    print('This command must be run from the engine/src directory, '
        'or be passed that directory as the --in parameter.\n');
    print(argParser.usage);
    exit(-1);
  }

  final Directory androidSdkDir = Directory(
    path.join(argResults['in'], 'third_party', 'android_tools', 'sdk'),
  );

  if (!androidSdkDir.existsSync()) {
    print('The Android SDK for this engine is missing from the '
        'third_party/android_tools directory. Have you run gclient sync?\n');
    print(argParser.usage);
    exit(-1);
  }

  final IOSink projectXml = File('./project.xml').openWrite();
  projectXml.write('''<!-- THIS FILE IS GENERATED. PLEASE USE THE INCLUDED DART PROGRAM  WHICH -->
<!-- WILL AUTOMATICALLY FIND ALL .java FILES AND INCLUDE THEM HERE       -->
<project>
  <sdk dir="${androidSdkDir.path}" />
  <module name="FlutterEngine" android="true" library="true" compile-sdk-version="android-P">
  <manifest file="${path.join(androidDir.path, 'AndroidManifest.xml')}" />
''');
  for (final FileSystemEntity entity in androidDir.listSync(recursive: true)) {
    if (!entity.path.endsWith('.java')) {
      continue;
    }
    projectXml.writeln('    <src file="${entity.path}" />');
  }

  projectXml.write('''  </module>
</project>
''');
  await projectXml.close();

  print('Wrote project.xml, starting lint...');
  const LocalProcessManager processManager = LocalProcessManager();
  final ProcessResult result = await processManager.run(
    <String>[
      path.join(androidSdkDir.path, 'tools', 'bin', 'lint'),
      '--project',
      './project.xml',
      '--html',
      argResults['out'],
      '--showall',
    ],
  );
  if (result.exitCode != 0) {
    print(result.stderr);
    exit(result.exitCode);
  }
  print(result.stdout);
  // TODO(dnfield): once we know what a clean lint will look like for this
  // project, we should detect it and set the exit code based on it.
  // Android Lint does _not_ set the exit code to non-zero if it detects lint
  // errors/warnings.
  return;
}
