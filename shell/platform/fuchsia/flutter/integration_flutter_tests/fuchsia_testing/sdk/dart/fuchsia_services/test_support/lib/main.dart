// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:fidl/fidl.dart';
import 'package:fidl_test_fuchsia_service_foo/fidl_async.dart' as foo;
import 'package:fuchsia_services/services.dart';

void main(List<String> args) {
  final context = ComponentContext.create();
  final fooImpl = _FooImpl();

  context.outgoing
    ..addPublicService(fooImpl.bind, foo.Foo.$serviceName)
    ..serveFromStartupInfo();
}

class _FooImpl extends foo.Foo {
  final _binding = foo.FooBinding();

  void bind(InterfaceRequest<foo.Foo> request) {
    _binding.bind(this, request);
  }

  @override
  Future<String> echo(String value) async => value;
}
