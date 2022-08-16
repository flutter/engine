# Unit testing Java code

All Java code in the engine should now be able to be tested with Robolectric 4.7.3
and JUnit 4. The test suite has been added after the bulk of the Java code was
first written, so most of these classes do not have existing tests. Ideally code
after this point should be tested, either with unit tests here or with
integration tests in other repos.

## Adding a new test

1. Create a file under `test/` matching the path and name of the class under
   test. For example,
   `shell/platform/android/io/flutter/util/Preconditions.java` ->
   `shell/platform/android/**test**/io/flutter/util/Preconditions**Test**.java`.
2. Write your test.
3. Build android embedding code `android_debug_unopt`.
3. Run the test with `testing/run_tests.py [--type=java] [--java-filter=<test_class_name>]`.

## Q&A

### My new test won't run. There's a "ClassNotFoundException".

See [Updating Embedding Dependencies](/tools/cipd/android_embedding_bundle).

### My new test won't compile. It can't find one of my imports.

See [Updating Embedding Dependencies](/tools/cipd/android_embedding_bundle).
