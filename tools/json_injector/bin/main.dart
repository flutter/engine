// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show JsonEncoder, jsonDecode, jsonEncode;
import 'dart:io';

import 'package:args/args.dart';
import 'package:json_injector/json_injector.dart';

void main(List<String> arguments) {
  final parser = ArgParser()
    ..addOption('input',
        abbr: 'i', help: 'Path to the input JSON file', mandatory: true)
    ..addOption('injector',
        abbr: 'j', help: 'Path to the injector JSON file', mandatory: true)
    ..addOption('output',
        abbr: 'o', help: 'Path to the output JSON file', mandatory: true)
    ..addOption('templates',
        abbr: 't', help: 'Path to the templates JSON file', mandatory: false)
    ..addOption('name-key',
        abbr: 'n', help: 'Name key for processing', mandatory: false);

  late final String inputJson;
  late final String injectorJson;
  late final String outputPath;

  ArgResults argResults;
  try {
    argResults = parser.parse(arguments);
    inputJson = File(argResults['input'] as String).readAsStringSync();
    injectorJson = File(argResults['injector'] as String).readAsStringSync();
    outputPath = argResults['output'] as String;
  } catch (e) {
    print('Error: $e\n');
    print('Usage:\n${parser.usage}');
    return;
  }

  final String? nameKey = argResults['name-key'] as String?;
  Map<dynamic, dynamic>? templates;
  final String? templatesPath = argResults['templates'] as String?;
  if (templatesPath != null) {
    final templateJson = File(templatesPath).readAsStringSync();
    templates = jsonDecode(templateJson) as Map<dynamic, dynamic>?;
  }

  late final dynamic input;
  try {
    input = jsonDecode(inputJson);
  } catch (ex) {
    print('failed to parse: ${argResults['input']}');
    print('Error: $ex');
    return;
  }
  late final dynamic injector;
  try {
    injector = jsonDecode(injectorJson);
  } catch (ex) {
    print('failed to parse: ${argResults['injector']}');
    print('Error: $ex');
    return;
  }

  final dynamic result =
      inject(input, injector, nameKey: nameKey, templates: templates);
  File(outputPath)
      .writeAsStringSync(const JsonEncoder.withIndent('  ').convert(result));
}
