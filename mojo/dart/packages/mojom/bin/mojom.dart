// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'package:mojom/src/command_runner.dart';
import 'package:mojom/src/utils.dart'
    show CommandLineError, GenerationError, FetchError;

main(List<String> arguments) async {
  var commandRunner = new MojomCommandRunner();
  try {
    return await commandRunner.run(arguments);
  } on CommandLineError catch (e) {
    stderr.writeln("$e\n${commandRunner.usage}");
  } on GenerationError catch (e) {
    stderr.writeln("$e\n${commandRunner.usage}");
  } on FetchError catch (e) {
    stderr.writeln("$e\n${commandRunner.usage}");
  }
}
