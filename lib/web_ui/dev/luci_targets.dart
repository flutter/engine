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
  Target(
    name: '//lib/web_ui:unit_tests_chrome_linux',
    agentProfiles: kLinuxAgent,
    runner: UnitTestsRunner(),
  ),
  Target(
    name: '//lib/web_ui:integration_tests_chrome_linux',
    agentProfiles: kLinuxAgent,
    runner: IntegrationTestsRunner(),
  ),
  Target(
    name: '//lib/web_ui:integration_tests_safari_mac',
    agentProfiles: kMacAgent,
    runner: IntegrationTestsRunner(),
  ),
  Target(
    name: '//lib/web_ui:check_licenses',
    agentProfiles: kLinuxAgent,
    runner: CheckLicensesRunner(),
  ),
];
