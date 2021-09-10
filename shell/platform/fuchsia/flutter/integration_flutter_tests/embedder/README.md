# `flutter scenic embedder tests`

<!-- TODO(richkadel): FIX OR REMOVE INVALID INSTRUCTIONS
-->
# THESE INSTRUCTIONS ARE A WORK IN PROGRESS. I'VE BEEN TOLD THIS MAY NOT WORK
# (FOR INSTANCE, DUE TO DART COMPILER DIFFERENCES) AND IF IT DOES WORK, THE
# CHANGES VALIDATED BETWEEN VERSIONS OF FLUTTER, FUCHSIA, AND FUCHSIA_SDK MAY
# STILL BE INCOMPATIBLE WITH OTHER DEPENDENT REPOSITORIES IN FUCHSIA'S RELEASE-
# AND-ROLL WORKFLOW. TAKE THESE WITH A GRAIN OF SALT FOR NOW, AS THEY EVOLVE.

To run the tests:

## Sync your fuchsia.git checkout to your current Fuchsia SDK version

First, sync your Fuchsia checkout and dependencies to match the versions used
to generate the Fuchsia SDK version you are using in the flutter/engine repo.
Typically, you've used `gclient sync` to download the version specified in the
flutter/engine `DEPS` file.

In a web browser, open
https://chrome-infra-packages.appspot.com/p/fuchsia/sdk/core, select your
architecture, click one of the "Instances" to open it's detail page, and then
edit the web URL to replace the selected instance ID with the instance ID you
synched with, from the DEPS file (for the same architecture).

<!-- TODO(richkadel): after any flutter_update, it would be nice if we could
warn the user if DEPS was updated after a recent roll of a newer Fuchsia SDK?
-->

From the new page matching your current instance ID, copy the `jiri_snapshot`
(only the value after the colon) and set a shell variable to this value:

```shell
$ JIRI_SNAPSHOT="<tag>" # for example, 6.20210809.3.1
```

Now, from your `$FUCHSIA_DIR` directory, do the following:

Use `git` to checkout the version of the `$FUCHSIA_DIR/integration` directory
matching your Fuchsia SDK's `$JIRI_SNAPSHOT`.

### Option 1: Checkout the matching tagged revision of Fuchsia, if available

Depending on your fuchsia.git configuration, you may be able to check it out by
tag:

```shell
$ cd "$FUCHSIA_DIR"
$ git -C integration checkout "tags/releases/$JIRI_SNAPSHOT"
```

### Option 2: Use git to find the matching revision of Fuchsia

But if your `$FUCHSIA_DIR/integration` git checkout doesn't have the release
tags, you may be able to find the right version by searching its git log
history:

```shell
$ cd "$FUCHSIA_DIR"
$ git -C integration log --grep="Roll sdk-core packages to version:$JIRI_SNAPSHOT"
commit abcdef1234567890abcdef123456789
Author: global-integration-roller ...
Date: ...

    [roll] Roll sdk-core packages to version:6.20210809.4.1
...
```

***IMPORANT: Your search may return more than one result. If so, choose one. Or
you may get no result (perhaps due to a manual roll, or other maintenance
activity). In this case, you may need to modify the `git log` `--grep=` pattern
and drop suffixes from your `$JIRI_SNAPSHOT` version to find a closely related
version. This would not guarantee compatibility, so if you're not a gambler, you
may want to update the SDK instance ID in your local `DEPS` file, and re-run
`gclient sync`, to ensure your installed `$FUCHSIA_SDK` matches the version
returned by `git log`.***

### Update fuchsia.git to match `integration`, and sync all dependencies

```shell
$ cd "$FUCHSIA_DIR"
$ git checkout "$(sed -ne 's/ *revision="\([^"]*\).*/\1/p' $(find integration -name stem) )"
$ fx sync-from-stem  # this will be very slow the first time, while it caches git history
```

The `fx sync-from-stem` command uses `jiri` to update all dependencies (other
`git` repos and CIPD prebuilts) to match the current $FUCHSIA_DIR commit (as set
by `git checkout ...` above). These versions should match the versions used when
creating the Fuchsia SDK you are using.

If you are also modifying Fuchsia, you may want to cherry-pick your changes on
top of your current checkout.

## Beware of Dart SDK differences between Flutter and Fuchsia

NOTE: These steps may or may not be necessary, and have not been confirmed
at this point.

To match Fuchsia's version of Dart, you may be able to update your DEPS file to
install a version of the Dart SDK that matches the version used in your
revision-synched `$FUCHSIA_DIR` prebuilts.

```shell
$ cat "$FUCHSIA_DIR/prebuilt/third_party/dart/linux-x64/version"
2.14.0-289.0.dev # for example
```

https://dart.googlesource.com/sdk/+/refs/tags/<the version tag from above>

You may be able to use the `tag` reference hash, to update the Dart SDK
dependency in `DEPS`, use `gclient sync` to download that version, and then
run `create_updated_flutter_deps.py` to update DEPS once more with the correct
versions of other dependencies required by the newly updated version of the
Dart SDK.

```shell
$ cd "$FLUTTER_ENGINE_DIR/src"
$ gclient sync
$ tools/dart/create_updated_flutter_deps.py
$ gclient sync
```

## Build flutter/engine:

```shell
$ cd "$FLUTTER_ENGINE_DIR/src"
$ ./flutter/tools/gn --fuchsia <flags> \
      # for example: --goma --fuchsia-cpu=x64 --runtime-mode=release
$ ninja -C out/fuchsia_release_x64
```

## If needed, replace Fuchsia's flutter_runner with your custom built runner

Follow the steps in the Flutter wiki, under
[Compiling for Fuchsia](https://github.com/flutter/flutter/wiki/Compiling-the-engine#compiling-for-fuchsia),
including the subsection titled
[Deploy to Fuchsia](https://github.com/flutter/flutter/wiki/Compiling-the-engine#deploy-to-fuchsia).

## Build and reboot your Fuchsia device, for example:

```shell
$ cd "$FUCHSIA_DIR"
$ # you may want to run `fx clean`, particularly right after `fx sync-from-stem`
$ fx set workstation.qemu-x64
$ fx build
$ fx reboot -r
```

<!--
$ fx pm publish -a -repo "$(cat ~/fuchsia/.fx-build-dir)/amber-files/" -f \
    "$FLUTTER_ENGINE_DIR"/src/out/fuchsia_*64/flutter-embedder-test-0.far
$ fx pm publish -a -repo "$(cat ~/fuchsia/.fx-build-dir)/amber-files/" -f \
    $(find "$FLUTTER_ENGINE_DIR"/src/out/fuchsia_*64 -name parent-view.far)
$ fx pm publish -a -repo "$(cat ~/fuchsia/.fx-build-dir)/amber-files/" -f \
    $(find "$FLUTTER_ENGINE_DIR"/src/out/fuchsia_*64 -name child-view.far)
-->

The test requires scenic, but the workstation UI consumes the scenic services,
exclusively. Kill scenic (which kills the workstation UI), run the flutter
package server, and then run the test.

```shell
$ fx shell killall scenic.cmx

$ flutter/tools/fuchsia/devshell/serve.sh --out out/fuchsia_debug_x64 --only-serve-runners

$ fx shell run-test-component \
    fuchsia-pkg://engine/flutter-embedder-test#meta/flutter-embedder-test.cmx
```

From here, you can modify the Flutter test, rebuild flutter, and usually rerun the test without
rebooting, by repeating the calls above to `fx pm ...` and `fx shell run-test-component ...`.

The embedder tests must be run on a product without a graphical base shell,
such as `core` because it starts and stops Scenic.
