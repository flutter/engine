#!/bin/bash

set -e

COMPILE_COMMANDS="out/compile_commands.json"
if [ ! -f $COMPILE_COMMANDS ]; then
  ./flutter/tools/gn
fi

exit 0
# This CI step was added after 1.20.x was cut
#dart flutter/ci/lint.dart $COMPILE_COMMANDS flutter/
