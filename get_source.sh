#!/bin/bash

get_depot_tools(){
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git -b main
  export PATH=$PATH:${PWD}/depot_tools
}

ENGINE_VERSION=$(curl -s https://raw.githubusercontent.com/flutter/flutter/stable/bin/internal/engine.version)

echo "ENGINE_VERSION=${ENGINE_VERSION}"

export DEPOT_TOOLS_UPDATE=0 
export GCLIENT_PY3=1
gclient --version

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

gclient sync --no-history --revision ${FLUTTER_ENGINE_SHA} -R -D -j ${NUM_PROC} 
