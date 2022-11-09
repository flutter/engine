// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io';

import 'package:args/args.dart';
import 'package:const_finder/const_finder.dart';

void main(List<String> args) {
  final ArgParser parser = ArgParser();
  parser
    ..addSeparator('Finds constant instances of a specified class from the\n'
        'specified package, and outputs JSON like the following:')
    ..addSeparator('''
  {
    "constantInstances": [
      {
        "codePoint": 59470,
        "fontFamily": "MaterialIcons",
        "fontPackage": null,
        "matchTextDirection": false
      }
    ],
    "nonConstantInstances": [
      {
        "file": "file:///Path/to/hello_world/lib/file.dart",
        "line": 19,
        "column": 11
      }
    ]
  }''')
    ..addSeparator('Where the "constantInstances" is a list of objects containing\n'
        'the properties passed to the const constructor of the class, and\n'
        '"nonConstantInstances" is a list of source locations of non-constant\n'
        'creation of the specified class. Non-constant creation cannot be\n'
        'statically evaluated by this tool, and callers may wish to treat them\n'
        'as errors. The non-constant creation may include entries that are not\n'
        'reachable at runtime.')
    ..addSeparator('Required arguments:')
    ..addOption('kernel-file',
        valueHelp: 'path/to/main.dill',
        help: 'The path to a kernel file to parse, which was created from the '
            'main-package-uri library.')
    ..addOption('class-library-uri',
        help: 'The package: URI of the class to find.',
        valueHelp: 'package:flutter/src/widgets/icon_data.dart')
    ..addOption('class-name',
        help: 'The class name for the class to find.', valueHelp: 'IconData')
    ..addSeparator('Optional arguments:')
    ..addFlag('pretty',
        negatable: false,
        help: 'Pretty print JSON output (defaults to false).')
    ..addFlag('help',
        abbr: 'h',
        negatable: false,
        help: 'Print usage and exit')
    ..addMultiOption('ignored-class-name',
      help: 'The name of a class whose containing constant instances that '
            'should not be retained. This is useful in situations--such as '
            'the web--where unused constants are not tree shaken from dill '
            'files.\n\nNote: each provided class name must pair with an '
            'invocation of "--ignored-class-library-uri" and appear in the '
            'same order.',
        valueHelp: 'Icons')
    ..addMultiOption('ignored-class-library-uri',
        help: 'The package: URI of a class to ignore.\n\nNote: each provided '
              'class name must pair with an invocation of '
              '"--ignored-class-name" and appear in the same order.',
        valueHelp: 'package:flutter/src/material/icons.dart');

  final ArgResults argResults = parser.parse(args);
  T getArg<T>(String name) {
    try {
      return argResults[name] as T;
    } catch (err) {
      stderr.writeln('Parsing error trying to parse the argument "$name"');
      rethrow;
    }
  }

  if (getArg<bool>('help')) {
    stdout.writeln(parser.usage);
    exit(0);
  }

  final List<List<String>> ignoredClasses = _pairIgnoredClassNamesAndUris(
    getArg<Object?>('ignored-class-name'),
    getArg<Object?>('ignored-class-library-uri'),
  );

  final ConstFinder finder = ConstFinder(
    kernelFilePath: getArg<String>('kernel-file'),
    classLibraryUri: getArg<String>('class-library-uri'),
    className: getArg<String>('class-name'),
    ignoredClasses: ignoredClasses,
  );

  final JsonEncoder encoder = getArg<bool>('pretty')
      ? const JsonEncoder.withIndent('  ')
      : const JsonEncoder();

  stdout.writeln(encoder.convert(finder.findInstances()));
}

List<List<String>> _pairIgnoredClassNamesAndUris(Object? names, Object? uris) {
  // If the user provided neither option then we will not ignore any classes.
  if (names == null && uris == null) {
    return const <List<String>>[];
  }
  // If the user provided one of the options but not the other, than this is a
  // error!
  if (names == null || uris == null) {
    stderr.writeln(
      'To ignore specific classes, both "--ignored-class-name" AND '
      '"--ignored-class-library-uri" must be provided in order to uniquely '
      'identify the class.',
    );
    exit(1);
  }

  names = names as List<String>;
  uris = uris as List<String>;
  final List<List<String>> pairs = <List<String>>[];

  if (names.length != uris.length) {
    stderr.writeln(
      '"--ignored-class-name" was provided ${names.length} time(s) but '
      '"--ignored-class-library-uri" was provided ${uris.length} time(s). '
      'Each ignored class name must be paired with exactly one ignored class '
      'library uri, provided in the same order.',
    );
    exit(1);
  }
  for (int i = 0; i < names.length; i++) {
    pairs.add(<String>[uris[i], names[i]]);
  }
  return pairs;
}
