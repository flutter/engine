#!/bin/bash

set -o pipefail -e;

HOST_DIR=${1:-host_debug_unopt}

# TODO(dnfield): Re-enable this when the upstream Dart changes that make it not be flaky land.
# out/$HOST_DIR/embedder_unittests
out/$HOST_DIR/flow_unittests
out/$HOST_DIR/fml_unittests --gtest_filter="-*TimeSensitiveTest*"
out/$HOST_DIR/runtime_unittests
out/$HOST_DIR/shell_unittests
out/$HOST_DIR/synchronization_unittests
out/$HOST_DIR/txt_unittests  --font-directory=flutter/third_party/txt/third_party/fonts

pushd flutter/testing/dart
pub get
popd

run_test () {
  out/$HOST_DIR/dart out/$HOST_DIR/gen/frontend_server.dart.snapshot --sdk-root out/$HOST_DIR/flutter_patched_sdk --incremental --strong --target=flutter --packages flutter/testing/dart/.packages --output-dill out/$HOST_DIR/engine_test.dill $1
  out/$HOST_DIR/flutter_tester --disable-observatory --use-test-fonts out/$HOST_DIR/engine_test.dill
}

# Verify that a failing test returns a failure code.
! run_test flutter/testing/smoke_test_failure/fail_test.dart

for TEST_SCRIPT in flutter/testing/dart/*.dart; do
  run_test $TEST_SCRIPT
done

pushd flutter
ci/test.sh
popd
exit 0
