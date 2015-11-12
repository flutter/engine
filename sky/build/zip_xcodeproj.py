#!/usr/bin/env python
#
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Creates a compressed archive of an Xcode project
"""

import optparse
import os
import sys
import zipfile

sys.path.append(os.path.join(os.path.dirname(__file__),
                os.pardir, os.pardir, 'build', 'android', 'gyp'))
from util import build_utils

EXCLUDE_DIRS = [
  "xcuserdata",
]

EXCLUDE_FILES = [
  ".DS_Store",
  ".pbxuser",
]

def ShouldExclude(dir, patterns):
  for pattern in patterns:
    if pattern in os.path.basename(os.path.normpath(dir)):
      return True
  return False

def ZipXcodeproj(args):
  with zipfile.ZipFile(args.output, 'w', zipfile.ZIP_DEFLATED) as outfile:
    for root, dirs, files in os.walk(args.input, topdown=True):
      dirs[:] = [dir for dir in dirs if not ShouldExclude(dir, EXCLUDE_DIRS) ]
      for file in files:
        if not(ShouldExclude(file, EXCLUDE_FILES)):
          zip_path = os.path.join(root, file)
          outfile.write(zip_path, os.path.relpath(zip_path, args.basepath))

def main():
  parser = optparse.OptionParser()
  build_utils.AddDepfileOption(parser)

  parser.add_option('--input', help='The xcode project directory to archive')
  parser.add_option('--output', help='Path to output archive')
  parser.add_option('--basepath', help='The base path in the archive')

  options, _ = parser.parse_args()

  ZipXcodeproj(options)
  build_utils.WriteDepfile(options.depfile, build_utils.GetPythonDependencies())

if __name__ == '__main__':
  sys.exit(main())
