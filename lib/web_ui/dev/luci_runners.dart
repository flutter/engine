// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// [TargetRunner] implmentations that run LUCI targets.

// @dart = 2.6
import 'environment.dart';
import 'luci_framework.dart';
import 'utils.dart';

class UnitTestsRunner implements TargetRunner {
  const UnitTestsRunner();

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

class IntegrationTestsRunner implements TargetRunner {
  const IntegrationTestsRunner();

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

class CheckLicensesRunner implements TargetRunner {
  const CheckLicensesRunner();

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
