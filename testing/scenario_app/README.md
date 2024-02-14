# Scenario App

This package simulates a Flutter app that uses the engine (`dart:ui`) only,
in conjunction with Android and iOS-specific embedding code that simulates the
use of the engine in a real app (such as plugins and platform views).

To run the tests, you will need to build the engine with the appropriate
configuration. The [`run_android_tests.sh`](run_android_tests.sh) and
[`run_ios_tests.sh`](run_ios_tests.sh) are then used to run the tests on a
connected device or emulator.

See also:

- [`bin/`](bin/), the entry point for running Android integration tests.
- [`lib/`](lib/), the Dart code and instrumentation for the scenario app.
- [`ios/`](ios/), the iOS-side native code and tests.
- [`android/`](android/), the Android-side native code and tests.

## Running a smoke test on Firebase TestLab

To run the smoke test on Firebase TestLab test, build `android_profile_arm64`,
and run [`./ci/firebase_testlab.py`](../../ci/firebase_testlab.py), or pass
`--variant` to run a different configuration.

```sh
# From the root of the engine repository
$ ./ci/firebase_testlab.py --variant android_debug_arm64
```

> ![NOTE]
> These instructions were not verified at the time of writing/refactoring.
