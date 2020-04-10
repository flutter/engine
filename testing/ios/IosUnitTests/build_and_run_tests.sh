#!/bin/sh

FLUTTER_ENGINE=ios_debug_sim_unopt

if [ $# -eq 1 ]; then
  FLUTTER_ENGINE=$1
fi

pushd $PWD
cd ../../../..
ninja -j 100 -C out/$FLUTTER_ENGINE ios_test_flutter
popd
./run_tests.sh $FLUTTER_ENGINE
