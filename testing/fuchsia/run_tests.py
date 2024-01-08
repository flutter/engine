#!/usr/bin/env python3
# Copyright (c) 2024, the Flutter project authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file.

import argparse
import os
import sys
from typing import List

sys.path.insert(
    0,
    os.path.join(
        os.path.dirname(__file__), '../../../fuchsia/test_scripts/test/'
    )
)

import run_test
from run_executable_test import ExecutableTestRunner
from test_runner import TestRunner


# TODO(zijiehe): Execute all the tests in
# https://github.com/flutter/engine/blob/main/testing/fuchsia/test_suites.yaml
def _get_test_runner(
    runner_args: argparse.Namespace, test_args: List[str]
) -> TestRunner:
  return ExecutableTestRunner(
      runner_args.out_dir, [],
      'fuchsia-pkg://fuchsia.com/dart_runner_tests#meta/dart_runner_tests.cm',
      runner_args.target_id, 'codecoverage', '/tmp/log',
      ['out/fuchsia_debug_x64/dart_runner_tests.far'], None
  )


# TODO(zijiehe): Respect build configurations.
if __name__ == '__main__':
  try:
    os.remove('out/fuchsia_debug_x64/dart_runner_tests.far')
  except FileNotFoundError:
    pass
  os.symlink(
      'dart_runner_tests-0.far', 'out/fuchsia_debug_x64/dart_runner_tests.far'
  )
  sys.argv.append('--out-dir=out/fuchsia_debug_x64')
  # The 'flutter-test-type' is a place holder and has no specific meaning; the
  # _get_test_runner is overrided.
  sys.argv.append('flutter-test-type')
  run_test._get_test_runner = _get_test_runner
  sys.exit(run_test.main())
