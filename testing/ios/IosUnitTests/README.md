# iOS Unit Tests

These are the unit tests for iOS engine.  They can be executed locally and are
also run in LUCI builds.

## Running Tests

```sh
./flutter/tools/gn --ios --simulator --unoptimized
cd flutter/testing/ios/IosUnitTests
./build_and_run_tests.sh
```

After the `ios_flutter_test` target is built you can also run the tests inside
of xcode with `IosUnitTests.xcodeproj`.

## Adding Tests

Add the test file to the source field of the
`//flutter/shell/platform/darwin/ios:ios_flutter_test` target.  Once it is there
it will execute with the others.
