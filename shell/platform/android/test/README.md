# Unit testing Java code

All Java code in the engine should now be able to be tested with Robolectric 3.8
and JUnit 4. The test suite has been added after the bulk of the Java code was
first written, so most of these classes do not have existing tests. Ideally code
after this point should be tested, either with unit tests here or with
integration tests in other repos.

## Adding a new test

1. Create a file under `test/` matching the path and name of the class under
   test. For example,
   `shell/platform/android/io/flutter/util/Preconditions.java` ->
   `shell/platform/android/**test**/io/flutter/util/Preconditions**Test**.java`.
2. Add your file to the `sources` of the `robolectric_tests` build target in
   `/shell/platform/android/BUILD.gn`. This compiles the test class into the
   test jar.
3. Add your class to the `@SuiteClasses` annotation in `FlutterTestSuite.java`.
   This makes sure the test is actually executed at run time.
4. Write your test.
5. Build and run with `testing/run_tests.py [--type=java] [filter=<test_class_name>]`.

## Q&A

### Why are we using Robolectric 3.8 when Robolectric 4+ is current?

Robolectric 4+ uses the AndroidX libraries, and the engine sources use the
deprecated android.support ones. See
[flutter/flutter#23586](https://github.com/flutter/flutter/issues/23586). If
this is an issue we could use Jetifier on `flutter.jar` first and _then_ run
the tests, but it would add an extra point of failure.

### My new test won't run. There's a "ClassNotFoundException".

Your test is probably using a dependency that we haven't needed yet. You
probably need to find the dependency you need, add it to the
`flutter/android/robolectric_bundle` CIPD package, and then re-run `gclient
sync`. See ["Updating a CIPD
dependency"](https://chromium.googlesource.com/chromium/src/+/master/docs/cipd.md#Updating-a-CIPD-dependency).

### My new test won't compile. It can't find one of my imports.

You could be using a brand new dependency. If so, first you'll want to grab the
jar following the same pattern as in the "My new test won't run" section.

Then you'll also need to add the jar to the `robolectric_tests` build target.
Add `//third_party/robolectric/lib/<dependency.jar>` to
`robolectric_tests._jar_dependencies` in `/shell/platform/android/BUILD.gn`.

There's also a chance that you're using a dependency that we're relying on at
runtime, but not compile time. If so you'll just need to update
`_jar_dependencies` in `BUILD.gn`.
