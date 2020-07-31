#!/bin/bash

set -e

# Needed because if it is set, cd may print the path it changed to.
unset CDPATH

# On Mac OS, readlink -f doesn't work, so follow_links traverses the path one
# link at a time, and then cds into the link destination and find out where it
# ends up.
#
# The returned filesystem path must be a format usable by Dart's URI parser,
# since the Dart command line tool treats its argument as a file URI, not a
# filename. For instance, multiple consecutive slashes should be reduced to a
# single slash, since double-slashes indicate a URI "authority", and these are
# supposed to be filenames. There is an edge case where this will return
# multiple slashes: when the input resolves to the root directory. However, if
# that were the case, we wouldn't be running this shell, so we don't do anything
# about it.
#
# The function is enclosed in a subshell to avoid changing the working directory
# of the caller.
function follow_links() (
  cd -P "$(dirname -- "$1")"
  file="$PWD/$(basename -- "$1")"
  while [[ -h "$file" ]]; do
    cd -P "$(dirname -- "$file")"
    file="$(readlink -- "$file")"
    cd -P "$(dirname -- "$file")"
    file="$PWD/$(basename -- "$file")"
  done
  echo "$file"
)
PROG_NAME="$(follow_links "${BASH_SOURCE[0]}")"
CI_DIR="$(cd "${PROG_NAME%/*}" ; pwd -P)"
SRC_DIR="$(cd "$CI_DIR/../.."; pwd -P)"

COMPILE_COMMANDS="$SRC_DIR/out/compile_commands.json"
if [ ! -f "$COMPILE_COMMANDS" ]; then
  (cd $SRC_DIR; ./flutter/tools/gn)
fi

cd "$CI_DIR"
pub get && dart \
  bin/lint.dart \
  --compile-commands="$COMPILE_COMMANDS" \
  --repo="$SRC_DIR/flutter" \
  "$@"
