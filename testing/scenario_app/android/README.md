# Scenario App: Android Tests

As mentioned in the [top-level README](../README.md), this directory contains
the Android-specific native code and tests for the [scenario app](../lib). To
run the tests, you will need to build the engine with the appropriate
configuration.

For example, `android_debug_unopt` or `android_debug_unopt_arm64` was built,
run:

```sh
# From the root of the engine repository
$ ./testing/run_android_tests.sh android_debug_unopt

# Or, for arm64
$ ./testing/run_android_tests.sh android_debug_unopt_arm64
```

## Updating Gradle dependencies

If a Gradle dependency is updated, lockfiles must be regenerated.

To generate new lockfiles, run:

```bash
cd android/app
../../../../../third_party/gradle/bin/gradle generateLockfiles
```
