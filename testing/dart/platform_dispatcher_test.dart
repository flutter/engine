// Copyright 2022 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'dart:ui';

import 'package:analyzer/dart/analysis/features.dart';
import 'package:analyzer/dart/analysis/results.dart';
import 'package:analyzer/dart/analysis/utilities.dart';
import 'package:analyzer/dart/ast/ast.dart';
import 'package:litetest/litetest.dart';

void main() {
  test('A ViewConfiguration asserts that both window and view are not provided', () {
    expectAssertion(() {
      return ViewConfiguration(
      // ignore: deprecated_member_use
        window: PlatformDispatcher.instance.views.first,
        view: PlatformDispatcher.instance.views.first,
      );
    });
  });

  test("A ViewConfiguration's view and window are backed with the same property", () {
    final FlutterView view = PlatformDispatcher.instance.views.first;
    final ViewConfiguration viewConfiguration = ViewConfiguration(view: view);
    // ignore: deprecated_member_use
    expect(viewConfiguration.window, view);
    // ignore: deprecated_member_use
    expect(viewConfiguration.window, viewConfiguration.view);
  });

  test('Initialize a ViewConfiguration with a window', () {
    final FlutterView view = PlatformDispatcher.instance.views.first;
    // ignore: deprecated_member_use
    ViewConfiguration(window: view);
  });

  test("copyWith() on a ViewConfiguration asserts that both a window aren't provided", () {
    final FlutterView view = PlatformDispatcher.instance.views.first;
    expectAssertion(() {
      // ignore: deprecated_member_use
      return ViewConfiguration(view: view)..copyWith(view: view, window: view);
    });
  });

  test("PlatformDispatcher and PlatformConfiguration have matching getters", () {
      final List<Pattern> exclusionCriteria = <Pattern>[
        RegExp(r'^_.*'), // exclude any private members
        RegExp(r'^on[A-Z].*'), // exclude any callbacks
        RegExp(r'locale$'), // locale is a convenience getter for interacting with `locales` which _is_ backed by the config
        'instance', // exclude the static instance of PlatformDispatcher
        'configuration', // the configuration should not reference itself
        'views', // views are backed by their own config and don't need to be referenced in the platform config
        'initialLifecycleState', // default route is stored/retrieved from the platform, not the engine
        'frameData', // framed data updates too often and would kick off too many config changes
      ];

      final String flutterDir = Platform.environment['FLUTTER_DIR']!;
      final List<ClassDeclaration> classes = parseFile(
        path: '$flutterDir/lib/ui/platform_dispatcher.dart',
        featureSet: FeatureSet.latestLanguageVersion(),
        throwIfDiagnostics: false,
      ).unit.declarations.whereType<ClassDeclaration>().toList();
      final ClassDeclaration dispatcherClass = classes.singleWhere((ClassDeclaration c) => c.name.toString() == (PlatformDispatcher).toString());
      final ClassDeclaration configurationClass = classes.singleWhere((ClassDeclaration c) => c.name.toString() == (PlatformConfiguration).toString());

      Iterable<String> getNameOfClassMember(ClassMember member) {
        if (member is MethodDeclaration) {
          return <String>[member.name.toString()];
        }else if (member is FieldDeclaration) {
          return member.fields.variables.map((VariableDeclaration variable) => variable.name.toString());
        } else {
          return [];
        }
      }

      final Set<String> dispatcherMembers = dispatcherClass.members
        .where((ClassMember member) =>
          member is FieldDeclaration ||
          member is MethodDeclaration && member.isGetter)
        .expand<String>(getNameOfClassMember)
        .where((String name) =>
          exclusionCriteria.every((criteria) =>
            criteria.allMatches(name).isEmpty))
        .toSet();
      final Set<String> configurationMembers = configurationClass.members
        .where((ClassMember member) =>
          member is FieldDeclaration ||
          member is MethodDeclaration && member.isGetter)
        .expand(getNameOfClassMember)
        .toSet();

      final Set<String> missingMembers = configurationMembers.difference(dispatcherMembers);
      final Set<String> extraMembers = dispatcherMembers.difference(configurationMembers);
      expect(missingMembers.isEmpty && extraMembers.isEmpty, isTrue,
        reason:
          'PlatformDispatcher is out of sync with PlatformConfiguration.\n'
          '  Missing Members: $missingMembers\n'
          '  Members not backed by PlatformConfiguration: $extraMembers');
  });
}
