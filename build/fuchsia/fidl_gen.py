#!/usr/bin/env python
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import json
import subprocess
import os
import argparse

def main():
  parser = argparse.ArgumentParser();

  parser.add_argument('--fidlc-bin', dest='fidlc_bin', action='store', required=True)
  parser.add_argument('--fidlgen-bin', dest='fidlgen_bin', action='store', required=True)
  
  parser.add_argument('--json', dest='json', action='store', required=True)
  parser.add_argument('--fidl-sources', dest='fidl_sources', nargs='+', action='store', required=True)
  parser.add_argument('--include-base', dest='include_base', action='store', required=True)
  parser.add_argument('--output-base', dest='output_base', action='store', required=True)

  args = parser.parse_args()

  assert os.path.exists(args.fidlc_bin)
  assert os.path.exists(args.fidlgen_bin)

  fidlc_command = [
    args.fidlc_bin,
    '--json',
    args.json,
    '--files'
  ] + args.fidl_sources

  subprocess.check_call(fidlc_command);

  assert os.path.exists(args.json)

  fidlgen_command = [
    args.fidlgen_bin,
    '-generators',
    'cpp',
    '-include-base',
    args.include_base,
    '-json',
    args.json,
    '-output-base',
    args.output_base
  ]

  subprocess.check_call(fidlgen_command)

  return 0

if __name__ == '__main__':
  sys.exit(main())
