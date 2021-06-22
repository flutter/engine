# syslog

Syslog provides logging infrastructure and corresponding tests for Fuchsia.

## Building tests

To add this component to your build, append
`--with-host-labels //sdk/lib/syslog/cpp:tests`
to the `fx set` invocation.

## Running tests

To run host tests, use
```
$ fx test --host logging_cpp_unittests
```


Note that host tests are not currently supported on ARM MacOS, but should function on Intel-based Macs or on certain Linux distributions.