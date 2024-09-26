#!/usr/bin/env python3
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Generates a shell or batch script to run a command."""

import argparse

def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--output', required=True, help='Output file')
  parser.add_argument('--command', required=True, help='Command to run')
  parser.add_argument('--args', required=True, help='Arguments to the command')
  args = parser.parse_args()

  with open(args.output, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('set -e\n')
    f.write('set -x\n')
    f.write(args.command)
    f.write(' ')
    f.write(args.args)
    f.write('\n')

  # Make the script executable.
  os.chmod(args.output, 0o755)

if __name__ == '__main__':
  main()
