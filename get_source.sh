#!/bin/bash

cat << EOF > .gclient
solutions = [
  {
    "managed": False,
    "name": "src/flutter",
    "url": "git@github.com:easion/engine.git@master",
    "custom_deps": {},
    "deps_file": "DEPS",
    "safesync_url": "",
  },
]
EOF

gclient sync

