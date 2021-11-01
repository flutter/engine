# Flutter-on-Fuchsia Integration Tests

## Configure and build fuchsia

Run `fx set` with a terminal image, which contains all required targets:

```shell
$ cd "$FUCHSIA_DIR"
$ fx set terminal.qemu-x64
$ fx build
```

## Restart and reboot your device

_(Optional)_ If developing with the emulator, launch (or shutdown and relaunch)
the emulator.

```shell
fx vdl start -N
```

NOTE: Do _not_ run the default package server. The instructions below describe
how to launch a flutter-specific package server.

Or if you've rebuilt fuchsia for a device that is already running a version of
fuchsia, you may be able to reboot without restarting the device:

```shell
$ fx reboot -r
```

## Build the tests

You can specify the test group target to build all integration tests.  Each
test builds and packages any runners it requires.

```shell
$ cd "$FLUTTER_ENGINE_DIR/src"
$ ./flutter/tools/gn --fuchsia --enable-unittests <flags> \
      # for example: --goma --fuchsia-cpu=x64 --runtime-mode=debug
$ ninja -C out/fuchsia_debug_x64 \
    flutter/shell/platform/fuchsia/flutter/integration_flutter_tests:tests
```

## Publish the test packages to the Fuchsia package server

The tests currently specify the Fuchsia package server's standard domain,
`fuchsia.com`, as the server to use to resolve (locate and load) the test
packages. So, before running the test, the most recently built `.far` files
need to be published to the Fuchsia package repo:

```shell
$ fx pm publish -a -repo "$(cat $FUCHSIA_DIR/.fx-build-dir)/amber-files/" \
  -f "$FLUTTER_ENGINE_DIR"/src/out/fuchsia_*64/gen/shell/platform/fuchsia/flutter/integration_flutter_tests/dart_runner/dart_runner_integration_tests/dart_runner_integration_tests.far
$ fx pm publish -a -repo "$(cat $FUCHSIA_DIR/.fx-build-dir)/amber-files/" \
  -f "$FLUTTER_ENGINE_DIR"/src/out/fuchsia_*64/gen/shell/platform/fuchsia/flutter/integration_flutter_tests/flutter_runner/flutter_runner_integration_tests/flutter_runner_integration_tests.far
```

## Run the test (using the package server at `fuchsia.com`)

```shell
$ fx shell run-test-component \
    fuchsia-pkg://fuchsia.com/dart_runner_integration_tests#meta/dart_runner_integration_tests.cmx
$ fx shell run-test-component \
    fuchsia-pkg://fuchsia.com/flutter_runner_integration_tests#meta/flutter_runner_integration_tests.cmx
```

## Make a change and re-run the test

If, for example, you only make a change to the Dart code in `parent_view`, you
can rebuild only the parent_view package target, republish it, and then re-run
the test, with:

```shell
$ ninja -C out/fuchsia_debug_x64 \
    flutter/shell/platform/fuchsia/flutter/integration_flutter_tests/flutter_runner
$ fx pm publish -a -repo "$(cat $FUCHSIA_DIR/.fx-build-dir)/amber-files/" \
  -f "$FLUTTER_ENGINE_DIR"/src/out/fuchsia_*64/gen/shell/platform/fuchsia/ \
flutter/integration_flutter_tests/flutter_runner/ \
flutter_runner_integration_tests/flutter_runner_integration_tests.far
$ fx shell run-test-component \
    fuchsia-pkg://fuchsia.com/flutter_runner_integration_tests#meta \
flutter_runner_integration_tests.cmx
```

By repeating the commands above, you can modify the Flutter test, rebuild
flutter, and rerun the test without rebooting.

## (Alternative) Serving flutter packages from a custom package server

If you want to use a custom package server, you will need to edit these sources:

    * `//flutter/shell/platform/fuchsia/flutter/integration_flutter_tests/
dart_runner/dart_runner_integration_test.cc`
    * `//flutter/shell/platform/fuchsia/flutter/integration_flutter_tests/
flutter_runner/flutter_runner_integration_test.cc`
    * `//flutter/shell/platform/fuchsia/flutter/integration_flutter_tests/
flutter_runner/parent-view2/parent_view2.dart`

Search for the component URLs with the `fuchsia.com` domain, and change the
domain to `engine`, which is the domain currently registered with the custom
package server in `//tools/fuchsia/devshell/serve.sh`.

WARNING: Be careful not to check in that change because CI requires using the
`fuchsia.com` domain/package server.

In this setup, the `flutter/engine` test packages are served from a separate
package server. The `flutter/engine` repo's `serve.sh` script launches this
secondary package server, and configures package URL rewrite rules to redirect
fuchsia's requests for flutter and dart packages from `fuchsia.com` to flutter's
package server instead.

**IMPORTANT:** _The flutter package server must be launched **after** the
default package server, because both `fx serve` and flutter's `serve.sh` set
package URL rewrite rules, and only the last one wins._

Launch each package server in a separate window or shell:

```shell
$ cd "${FUCHSIA_DIR}"
$ fx serve
```

From the flutter engine `src` directory, run the following script to launch the
`engine` package server, to serve the flutter runner and test components.

```shell
$ flutter/tools/fuchsia/devshell/serve.sh --out out/fuchsia_debug_x64 \
--only-serve-runners
```

## Run the test, using `engine` as the package server domain

```shell
$ fx shell run-test-component \
    fuchsia-pkg://engine/dart_runner_integration_tests#meta/ \
dart_runner_integration_tests.cmx
$ fx shell run-test-component \
    fuchsia-pkg://engine/flutter_runner_integration_tests#meta/ \
flutter_runner_integration_tests.cmx
```

You can recompile and run the test without needing to re-publish the `.far`.
