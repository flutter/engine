// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'luci_framework.dart';
import 'luci_runners.dart';

/// The list of available targets.
///
/// Using GN-esque/Bazel-esque format so if we ever move to one of those
/// there's not a lot of relearning to do, but even if we don't at least
/// we'd be using familiar concepts.
const List<Target> targets = <Target>[
  // Runs unit-tests on Linux using Chrome.
  Target(
    name: '//lib/web_ui:unit_tests_linux_chrome',
    agentProfiles: kLinuxAgent,
    runner: WebUnitTestRunner(
      browser: 'chrome',
    ),
    environment: <String, String>{
      'CHROME_NO_SANDBOX': 'true',
    },
  ),

  // Runs unit-tests on Linux using Firefox.
  Target(
    name: '//lib/web_ui:unit_tests_linux_firefox',
    agentProfiles: kLinuxAgent,
    runner: WebUnitTestRunner(
      browser: 'firefox',
    ),
  ),

  // Runs e2e tests on Linux using Chrome.
  Target(
    name: '//e2etests/web:integration_tests_linux_chrome',
    agentProfiles: kLinuxAgent,
    runner: WebIntegrationTestsRunner(
      browser: 'chrome',
    ),
    environment: <String, String>{
      'CHROME_NO_SANDBOX': 'true',
    },
  ),

  // Checks license headers in Web engine sources.
  Target(
    name: '//lib/web_ui:check_licenses',
    agentProfiles: kLinuxAgent,
    runner: WebCheckLicensesRunner(),
  ),
];
