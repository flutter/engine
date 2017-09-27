Function Profiler
=================

To trace all the functions of a particular target, add the `//flutter/function_profiler` target as a `deps` member of that target. Then to its `cflags`, add the `-finstrument-functions` flag. All the functions will then end up in the timeline under the `func-profiler` category.
