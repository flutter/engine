// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_frontend_server/server.dart';
import 'package:frontend_server/frontend_server.dart' as frontend show ProgramTransformer;

import 'package:kernel/kernel.dart';
import 'package:mockito/mockito.dart';
import 'package:test/test.dart';

void main() async {
  const Set<String> uiAndFlutter = <String>{
    'dart:ui',
    'package:flutter',
  };

  test('No packages', () {
    final ToStringTransformer transformer = ToStringTransformer(null, <String>{});

    final MockComponent component = MockComponent();
    transformer.transform(component);
    verifyNever(component.visitChildren(any));
  });

  test('dart:ui package', () {
    final ToStringTransformer transformer = ToStringTransformer(null, uiAndFlutter);

    final MockComponent component = MockComponent();
    transformer.transform(component);
    verify(component.visitChildren(any)).called(1);
  });

  test('Child transformer', () {
    final MockTransformer childTransformer = MockTransformer();
    final ToStringTransformer transformer = ToStringTransformer(childTransformer, <String>{});

    final MockComponent component = MockComponent();
    transformer.transform(component);
    verifyNever(component.visitChildren(any));
    verify(childTransformer.transform(component)).called(1);
  });

  test('ToStringVisitor ignores non-toString procedures', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    when(procedure.name).thenReturn(Name('main'));
    when(procedure.annotations).thenReturn(const <Expression>[]);

    visitor.visitProcedure(procedure);
    verifyNever(procedure.enclosingLibrary);
  });

  test('ToStringVisitor ignores top level toString', () {
    // i.e. a `toString` method specified at the top of a library, like:
    //
    // void main() {}
    // String toString() => 'why?';
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final MockStatement statement = MockStatement();
    final Library library = Library(Uri.parse('package:some_package/src/blah.dart'));
    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(Name('toString'));
    when(procedure.annotations).thenReturn(const <Expression>[]);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(null);
    when(procedure.isAbstract).thenReturn(false);
    when(procedure.isStatic).thenReturn(false);
    when(function.body).thenReturn(statement);

    visitor.visitProcedure(procedure);
    verifyNever(statement.replaceWith(any));
  });

  test('ToStringVisitor ignores abstract toString', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final MockStatement statement = MockStatement();
    final Library library = Library(Uri.parse('package:some_package/src/blah.dart'));
    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(Name('toString'));
    when(procedure.annotations).thenReturn(const <Expression>[]);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());
    when(procedure.isAbstract).thenReturn(true);
    when(procedure.isStatic).thenReturn(false);
    when(function.body).thenReturn(statement);

    visitor.visitProcedure(procedure);
    verifyNever(statement.replaceWith(any));
  });

  test('ToStringVisitor ignores static toString', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final MockStatement statement = MockStatement();
    final Library library = Library(Uri.parse('package:some_package/src/blah.dart'));
    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(Name('toString'));
    when(procedure.annotations).thenReturn(const <Expression>[]);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());
    when(procedure.isAbstract).thenReturn(false);
    when(procedure.isStatic).thenReturn(true);
    when(function.body).thenReturn(statement);

    visitor.visitProcedure(procedure);
    verifyNever(statement.replaceWith(any));
  });

  test('ToStringVisitor ignores non-specified libraries', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final MockStatement statement = MockStatement();
    final Library library = Library(Uri.parse('package:some_package/src/blah.dart'));
    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(Name('toString'));
    when(procedure.annotations).thenReturn(const <Expression>[]);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());
    when(procedure.isAbstract).thenReturn(false);
    when(procedure.isStatic).thenReturn(false);
    when(function.body).thenReturn(statement);

    visitor.visitProcedure(procedure);
    verifyNever(statement.replaceWith(any));
  });

  test('ToStringVisitor ignores @pragma("flutter_frontend:keep")', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final MockStatement statement = MockStatement();
    final Library library = Library(Uri.parse('dart:ui'));
    final Name name = Name('toString');
    final Class pragmaClass = Class(name: 'pragma')..parent = Library(Uri.parse('dart:core'));

    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(name);
    when(procedure.annotations).thenReturn(<Expression>[
      ConstantExpression(
        InstanceConstant(
          Reference()..node = pragmaClass,
          <DartType>[],
          <Reference, Constant>{Reference(): StringConstant('flutter_frontend:keep')},
        ),
      ),
    ]);

    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());
    when(procedure.isAbstract).thenReturn(false);
    when(procedure.isStatic).thenReturn(false);
    when(function.body).thenReturn(statement);

    visitor.visitProcedure(procedure);
    verifyNever(statement.replaceWith(any));
  });

  void _validateReplacement(MockStatement body) {
    final ReturnStatement replacement = verify(body.replaceWith(captureAny)).captured.single as ReturnStatement;
    expect(replacement.expression, isA<SuperMethodInvocation>());
    final SuperMethodInvocation superMethodInvocation = replacement.expression as SuperMethodInvocation;
    expect(superMethodInvocation.name.name, 'toString');
  }

  test('ToStringVisitor replaces toString in specified libraries (dart:ui)', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final MockStatement statement = MockStatement();
    final Library library = Library(Uri.parse('dart:ui'));
    final Name name = Name('toString');

    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(name);
    when(procedure.annotations).thenReturn(const <Expression>[]);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());
    when(procedure.isAbstract).thenReturn(false);
    when(procedure.isStatic).thenReturn(false);
    when(function.body).thenReturn(statement);

    visitor.visitProcedure(procedure);
    _validateReplacement(statement);
  });

  test('ToStringVisitor replaces toString in specified libraries (package:flutter)', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final MockStatement statement = MockStatement();
    final Library library = Library(Uri.parse('package:flutter/src/foundation.dart'));
    final Name name = Name('toString');

    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(name);
    when(procedure.annotations).thenReturn(const <Expression>[]);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());
    when(procedure.isAbstract).thenReturn(false);
    when(procedure.isStatic).thenReturn(false);
    when(function.body).thenReturn(statement);

    visitor.visitProcedure(procedure);
    _validateReplacement(statement);
  });
}

class MockComponent extends Mock implements Component {}
class MockTransformer extends Mock implements frontend.ProgramTransformer {}
class MockProcedure extends Mock implements Procedure {}
class MockFunctionNode extends Mock implements FunctionNode {}
class MockStatement extends Mock implements Statement {}
