// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'dart:ui';

import 'package:test/test.dart';

String top() => "top";

class Foo {
  const Foo();
  static int getInt() => 1;
  double getDouble() => 1.0;
}

const Foo foo = const Foo();

void main() {
  test('PluginUtilities.getNameOf*', () {
    expect(PluginUtilities.getNameOfFunction(top), "top");
    expect(PluginUtilities.getNameOfFunction(Foo.getInt), "getInt");
    expect(PluginUtilities.getNameOfFunction(foo.getDouble), 'getDouble');
    expect(() => PluginUtilities.getNameOfFunction(null),
        throwsA(const isInstanceOf<ArgumentError>()));

    expect(PluginUtilities.getNameOfFunctionClass(top), null);
    expect(PluginUtilities.getNameOfFunctionClass(Foo.getInt), 'Foo');
    expect(() => PluginUtilities.getNameOfFunctionClass(null),
        throwsA(const isInstanceOf<ArgumentError>()));

    // Can't lookup class name for instance functions.
    expect(PluginUtilities.getNameOfFunctionClass(foo.getDouble), isNull);
  });

  test('PluginUtilities.getPathForFunctionLibrary', () {
    const String file = 'plugin_utilities_test.dart';
    expect(
        PluginUtilities.getPathForFunctionLibrary(top).endsWith(file), isTrue);
    expect(PluginUtilities.getPathForFunctionLibrary(Foo.getInt).endsWith(file),
        isTrue);
    expect(
        PluginUtilities.getPathForFunctionLibrary(foo.getDouble).endsWith(file),
        isTrue);

    expect(() => PluginUtilities.getPathForFunctionLibrary(null),
        throwsA(const isInstanceOf<ArgumentError>()));
  });

  test('PluginUtilities.getClosureByName', () {
    final String getIntName = PluginUtilities.getNameOfFunction(Foo.getInt);
    final String getDoubleName =
        PluginUtilities.getNameOfFunction(foo.getDouble);
    final String topName = PluginUtilities.getNameOfFunction(top);
    final String fooName = PluginUtilities.getNameOfFunctionClass(Foo.getInt);
    final String libName = PluginUtilities.getPathForFunctionLibrary(top);

    // We should successfully get a closure for top level and static methods.
    expect(
        PluginUtilities.getClosureByName(
            name: getIntName, libraryPath: libName, className: fooName)(),
        1);
    expect(
        PluginUtilities.getClosureByName(name: topName, libraryPath: libName)(),
        'top');

    // We don't support getting closures for instance methods.
    expect(
        PluginUtilities.getClosureByName(
            name: getDoubleName, libraryPath: libName, className: fooName),
        null);

    // Try an invalid function name.
    expect(PluginUtilities.getClosureByName(name: 'baz'), null);
    expect(() => PluginUtilities.getClosureByName(),
        throwsA(const isInstanceOf<ArgumentError>()));

    // Lookup from root library.
    expect(
        PluginUtilities.getClosureByName(
            name: getIntName, className: fooName)(),
        1);
    expect(PluginUtilities.getClosureByName(name: topName)(), 'top');
    expect(
        PluginUtilities.getClosureByName(
            name: getDoubleName, className: fooName),
        null);
  });
}
