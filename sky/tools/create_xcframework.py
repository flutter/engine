#!/usr/bin/env python
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import errno
import os
import shutil
import subprocess
import sys

def main():
  parser = argparse.ArgumentParser(
      description='Creates an XCFramework consisting of the specified universal frameworks')

  parser.add_argument('--frameworks',
    nargs='+', help='The framework paths used to create the XCFramework.', required=True)
  parser.add_argument('--name', help='Name of the XCFramework', type=str, required=True)
  parser.add_argument('--location', help='Output directory', type=str, required=True)

  args = parser.parse_args()

  output_dir = os.path.abspath(args.location)
  output_xcframework = os.path.join(output_dir, '%s.xcframework' % args.name)

  try:
    os.makedirs(output_dir)
  except OSError as e:
    # Ignore only "file exists" errors.
    if e.errno != errno.EEXIST:
      raise e

  try:
    # Remove old xcframework.
    shutil.rmtree(output_xcframework)
  except OSError as e:
    # Ignore only "not found" errors.
    if e.errno != errno.ENOENT:
      raise e

  # xcrun xcodebuild -create-xcframework -framework foo/baz.framework -framework bar/baz.framework -output output/
  command = ['xcrun',
    'xcodebuild',
    '-quiet',
    '-create-xcframework']

  [command.extend(['-framework', os.path.abspath(framework)]) for framework in args.frameworks]
  command.extend(['-output', output_xcframework])

  subprocess.check_call(command, stdout=open(os.devnull, 'w'))

if __name__ == '__main__':
  sys.exit(main())
