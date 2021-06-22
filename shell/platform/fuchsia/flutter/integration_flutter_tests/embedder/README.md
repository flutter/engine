# `flutter embedder tests`


To run the tests

```
fx set core.<board> --with //bundles:tests --with-base //topaz/bundles:buildbot
fx build
fx reboot -r
fx test flutter-embedder-test
```

The embedder tests must be run on a product without a graphical base shell,
such as `core` because it starts and stops Scenic.