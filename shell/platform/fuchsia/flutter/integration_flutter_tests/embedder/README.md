# `flutter scenic embedder tests`

## Build and reboot your Fuchsia device, for example:

```shell
$ cd "$FUCHSIA_DIR"
$ fx set <your build arguments> --with //src/ui/scenic
  # For example: fx set core.x64 --with //src/ui/scenic
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

If you are building a device that launches the UI at startup, you will likely
need to kill Scenic before running the test.

```shell
$ fx shell killall scenic.cmx
```

## Build the test

You can specify the test's package target to build only the test package, with
its dependencies. This will also build the required runner.

```shell
$ cd "$FLUTTER_ENGINE_DIR/src"
$ ./flutter/tools/gn --fuchsia <flags> \
      # for example: --goma --fuchsia-cpu=x64 --runtime-mode=debug
$ ninja -C out/fuchsia_debug_x64 \
    flutter/shell/platform/fuchsia/flutter/integration_flutter_tests/embedder/parent-view:package
```

## Start the flutter/engine-configured package server

Run the following script to launch a package server that will serve the
flutter runner that was built from the `flutter/engine` repo, when requested by
fuchsia, in order to execute the test. Since fuchsia looks for the flutter
runner at the `fuchsia.com` package server domain, the `serve.sh` script,
referenced below, remaps `fuchsia.com` to the `flutter/engine`-specific package
server (using the name `engine`), but only for packages using one of the
expected flutter runner package names. All other `fuchsia.com` requests map
back to the default `fuchsia.com` resolution (`devhost`).

```shell
$ flutter/tools/fuchsia/devshell/serve.sh --out out/fuchsia_debug_x64 --only-serve-runners
```

## Run the test, using `engine` as the package server domain

```shell
$ fx shell run-test-component \
    fuchsia-pkg://engine/flutter-embedder-test#meta/flutter-embedder-test.cmx
```

## Make a change and re-run the test

```shell
$ ninja -C out/fuchsia_debug_x64 \
    flutter/shell/platform/fuchsia/flutter/integration_flutter_tests/embedder/parent-view:package
$ fx shell run-test-component \
    fuchsia-pkg://engine/flutter-embedder-test#meta/flutter-embedder-test.cmx
```

From here, you can modify the Flutter test, rebuild flutter, and usually rerun the test without
rebooting, by repeating the calls above to `fx pm ...` and `fx shell run-test-component ...`.

The embedder tests must be run on a product without a graphical base shell,
such as `core` because it starts and stops Scenic.
