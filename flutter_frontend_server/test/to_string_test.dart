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
    final Library library = Library(Uri.parse('package:some_package/src/blah.dart'));
    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(Name('toString'));
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(null);

    visitor.visitProcedure(procedure);
    verifyNever(function.replaceWith(any));
  });

  test('ToStringVisitor ignores non-specified libraries', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final Library library = Library(Uri.parse('package:some_package/src/blah.dart'));
    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(Name('toString'));
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());

    visitor.visitProcedure(procedure);
    verifyNever(function.replaceWith(any));
  });

  void _validateReplacement(MockFunctionNode function) {
    final FunctionNode replacement = verify(function.replaceWith(captureAny)).captured.single as FunctionNode;
    expect(replacement.body, isA<ReturnStatement>());
    final ReturnStatement returnStatement = replacement.body as ReturnStatement;
    expect(returnStatement.expression, isA<SuperMethodInvocation>());
    final SuperMethodInvocation superMethodInvocation = returnStatement.expression as SuperMethodInvocation;
    expect(superMethodInvocation.name.name, 'toString');
  }

  test('ToStringVisitor replaces toString in specified libraries (dart:ui)', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final Library library = Library(Uri.parse('dart:ui'));
    final Name name = Name('toString');

    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(name);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());

    visitor.visitProcedure(procedure);
    _validateReplacement(function);
  });

  test('ToStringVisitor replaces toString in specified libraries (package:flutter)', () {
    final ToStringVisitor visitor = ToStringVisitor(uiAndFlutter);
    final MockProcedure procedure = MockProcedure();
    final MockFunctionNode function = MockFunctionNode();
    final Library library = Library(Uri.parse('package:flutter/src/foundation.dart'));
    final Name name = Name('toString');

    when(procedure.function).thenReturn(function);
    when(procedure.name).thenReturn(name);
    when(procedure.enclosingLibrary).thenReturn(library);
    when(procedure.enclosingClass).thenReturn(Class());

    visitor.visitProcedure(procedure);
    _validateReplacement(function);
  });
}

class MockComponent extends Mock implements Component {}
class MockTransformer extends Mock implements frontend.ProgramTransformer {}
class MockProcedure extends Mock implements Procedure {}
class MockFunctionNode extends Mock implements FunctionNode {}
