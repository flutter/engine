// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// [TargetRunner] implmentations that run LUCI targets.

// @dart = 2.6
import 'luci_common.dart';
import 'luci_framework.dart';

/// Runs Web unit-tests.
class WebUnitTestRunner implements TargetRunner {
  /// Creates a Web unit-test runner.
  const WebUnitTestRunner();

  @override
  Future<void> run(Target target) async {
    await runProcess(
      environment.feltExecutable,
      <String>[
        'test',
        '--unit-tests-only',
      ],
      workingDirectory: environment.webUiRootDir.path,
      mustSucceed: true,
    );
  }
}

/// Runs Web integration tests.
class WebIntegrationTestsRunner implements TargetRunner {
  /// Creates a Web integration test runner.
  const WebIntegrationTestsRunner();

  @override
  Future<void> run(Target target) async {
    await runProcess(
      environment.feltExecutable,
      <String>[
        'test',
        '--integration-tests-only',
      ],
      workingDirectory: environment.webUiRootDir.path,
      mustSucceed: true,
    );
  }
}

/// Checks license headers in Web Dart sources.
class WebCheckLicensesRunner implements TargetRunner {
  /// Creates a Web license header checker.
  const WebCheckLicensesRunner();

  @override
  Future<void> run(Target target) async {
    await runProcess(
      environment.feltExecutable,
      <String>['check-licenses'],
      workingDirectory: environment.webUiRootDir.path,
      mustSucceed: true,
    );
  }
}
