#!/usr/bin/env python

# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import subprocess
import sys

EMSDK_ROOT = os.path.abspath(
    os.path.join(__file__, '..', '..', '..', '..', 'buildtools', 'emsdk')
)

WASM_OPT_PATH = os.path.join(EMSDK_ROOT, 'upstream', 'bin', 'wasm-opt')


def main():
  parser = argparse.ArgumentParser(
      description='Call wasm-opt on a .wasm file'
  )
  parser.add_argument('--input', type=str, required=True)
  parser.add_argument('--output', type=str, required=True)

  args = parser.parse_args()
  try:
    subprocess.check_call([
        WASM_OPT_PATH, '-O3', args.input, '-o', args.output,
    ],
                          stdout=subprocess.DEVNULL)
  except subprocess.CalledProcessError:
    print('Failed to run wasm-opt')
    return 1


if __name__ == '__main__':
  sys.exit(main())
