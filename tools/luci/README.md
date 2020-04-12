# LUCI build scripts

## bin/luci.dart

A command-line tool for running LUCI build targets. This tool must be run using
the Dart SDK built as part of `host_debug_unopt`.

`host_debug_unopt` is the only dependency. `luci.dart` should be completely
self-contained otherwise. It should run `pub get` and invoke compilers as
necessary. If it doesn't, it's a bug.

Use `dart luci.dart targets` to enumerate available targets. The output is JSON
encoded. Adding `--pretty` will output in human-readable format.

Use `dart luci.dart run //path/to:target` to run a target named
`//path/to:target`. You can specify multiple targets by space-separating them.

## Target specification

All targets are defined in `lib/luci_targets.dart` using plain Dart. The targets
use concepts common in other build systems, such as GN, Bazel, Buck, and Please.

Example:

```dart
Target(
  // A unique name for the target. This name can be passed to `luci.dart run`
  // to run it.
  name: '//lib/web_ui:unit_tests_chrome_linux',

  // Agent profile that can run this target. Typically this will have one
  // profile, but in rare circumstances multiple profiles can be provided.
  // LUCI will find the build agent with the requested profile and run this
  // target in it.
  //
  // For convenience, `kLinuxAgent`, `kMacAgent`, and `kWindowsAgent` constants
  // are available.
  agentProfiles: <String>['linux'],

  // The runner that will run this target.
  runner: WebUnitTestRunner(),
)
```

The `//path/to:target` naming convention is currently not significant but is
recommended for consistency with other build systems and for discoverability.
For example, `//lib/web_ui` makes it very clear that the target has something
to do with the `lib/web_ui` directory. The convention may become significant
in the future.
