#!/bin/bash

# This script will regenerate spirv files for each file
# ending in glsl inside the test/goldens/src directory.
#
# It requires the following binaries to be in your PATH:
# - glslangValidator
# - spirv-opt

for src in test/goldens/src/*.glsl
do
  base=$(basename "$src" ".glsl")
  dst="test/goldens/$base.spv"
  glslangValidator -G100 -S frag -o $dst $src
  spirv-opt --strip-debug $dst -o $dst
done

