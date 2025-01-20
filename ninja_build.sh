#!/bin/bash
set -e
RED='\033[0;31m'
NOCOLOR='\033[0m'

if [[ $(uname -m) == "arm64" ]]; then
	echo "Host: arm64"
	GN_SIM_ARGS="--simulator-cpu=arm64"
	GN_ARGS="--mac-cpu=arm64"
	OUTPUT_POSTFIX="_arm64"
else 
	echo "Host: x64"
	GN_SIM_ARGS=""
	GN_ARGS=""
	OUTPUT_POSTFIX=""
fi

if [[ "$1" == "clean" ]]; then
	echo "Clean build ..."
	rm -irf ./out/ios_debug_sim_unopt$OUTPUT_POSTFIX
	rm -rf ./out/ios_debug_unopt$OUTPUT_POSTFIX
	rm -rf ./out/ios_release$OUTPUT_POSTFIX
	rm -rf ./out/host_debug_unopt$OUTPUT_POSTFIX
	rm -rf ./out/host_release$OUTPUT_POSTFIX
fi

if [[ "$1" == "clean" ]] || [[ ! -d ./out/ios_debug_sim_unopt$OUTPUT_POSTFIX ]]; then
   	./flutter/tools/gn --ios --no-goma --simulator --unoptimized $GN_SIM_ARGS
fi
ninja -C out/ios_debug_sim_unopt$OUTPUT_POSTFIX

if [[ "$1" == "clean" ]] || [[ ! -d ./out/ios_debug_unopt$OUTPUT_POSTFIX ]]; then
	./flutter/tools/gn --ios --no-goma --unoptimized $GN_ARGS
fi
ninja -C out/ios_debug_unopt$OUTPUT_POSTFIX

if [[ "$1" == "clean" ]] || [[ ! -d ./out/ios_release$OUTPUT_POSTFIX ]]; then
	./flutter/tools/gn --ios --no-goma --runtime-mode=release $GN_ARGS
fi
ninja -C out/ios_release$OUTPUT_POSTFIX

if [[ "$1" == "clean" ]] || [[ ! -d ./out/host_debug_unopt$OUTPUT_POSTFIX ]]; then
	./flutter/tools/gn --no-goma --unoptimized --no-prebuilt-dart-sdk $GN_ARGS
fi
ninja -C out/host_debug_unopt$OUTPUT_POSTFIX

if [[ "$1" == "clean" ]] || [[ ! -d ./out/host_release$OUTPUT_POSTFIX ]]; then
	./flutter/tools/gn --no-goma --no-lto --runtime-mode=release --no-prebuilt-dart-sdk $GN_ARGS
fi
ninja -C out/host_release$OUTPUT_POSTFIX