#!/bin/bash

if [[ -z "$1" ]]; then
    echo "Usage: $(basename $0) <engine_commit_hash>"
    exit 1
fi

ENGINE_COMMIT=$1
BUILDERS=$(curl 'https://ci.chromium.org/p/flutter/g/engine/builders' 2>/dev/null|sed -En 's:.*aria-label="builder buildbucket/luci\.flutter\.prod/([^/]+)".*:\1:p'|sort|uniq)

# This property is only for hotfixes to branches prior to this feature being added
# See https://github.com/flutter/engine/commit/abaac56c602cad3c1b3e41633bbfbe65200a1a3a
NO_FONT_SUBSET="-p build_font_subset=false"

IFS=$'\n'
for BUILDER in $BUILDERS; do
    echo "Building $BUILDER..."
    bb add \
       -commit "https://chromium.googlesource.com/external/github.com/flutter/engine/+/$ENGINE_COMMIT" \
       "flutter/prod/$BUILDER" \
       $NO_FONT_SUBSET
    sleep 1
done
