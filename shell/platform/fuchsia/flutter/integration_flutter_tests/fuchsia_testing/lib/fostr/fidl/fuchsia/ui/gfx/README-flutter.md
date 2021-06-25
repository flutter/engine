<!-- TODO(richkadel): replace this with something generated somehow? -->

The files in this directory were generated from the fuchsia
in-tree tool `fidlmerge`.

Since `fidlmerge` is not in the SDK, I am copying the files into the flutter engine repo for now.

Refreshed after building a compatible version of Fuchsia via:

```shell
$ cp ~/fuchsia/out/workstation.qemu-x64/gen/garnet/public/lib/fostr/fidl/fuchsia/ui/gfx/formatting.{cc,h} \
    flutter/shell/platform/fuchsia/flutter/integration_flutter_tests/fuchsia_testing/lib/fostr/fidl/fuchsia/ui/gfx/
```
