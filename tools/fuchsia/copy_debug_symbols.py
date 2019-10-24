#!/usr/bin/env python
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
""" Gather the build_id, prefix_dir, and exec_name given the path to executable
    also copies to the specified destination.
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys


def Touch(fname):
  with open(fname, 'a'):
    os.utime(fname, None)


def GetBuildIdParts(exec_path):
  file_out = subprocess.check_output(['file', exec_path])
  build_id = re.match('.*=(.*?),', file_out).groups()[0]
  return {
      'build_id': build_id,
      'prefix_dir': build_id[:2],
      'exec_name': build_id[2:]
  }


def main():
  parser = argparse.ArgumentParser()

  parser.add_argument(
      '--executable-name', dest='exec_name', action='store', required=True)
  parser.add_argument(
      '--executable-path', dest='exec_path', action='store', required=True)
  parser.add_argument(
      '--destination-base', dest='dest', action='store', required=True)

  parser.add_argument('--stripped', dest='stripped', action='store_true')
  parser.add_argument('--unstripped', dest='stripped', action='store_false')
  parser.set_defaults(stripped=True)

  args = parser.parse_args()
  assert os.path.exists(args.exec_path)
  assert os.path.exists(args.dest)

  parts = GetBuildIdParts(args.exec_path)
  dbg_prefix_base = '%s/%s' % (args.dest, parts['prefix_dir'])

  if not os.path.exists(dbg_prefix_base):
    os.makedirs(dbg_prefix_base)

  dbg_suffix = ''
  if not args.stripped:
    dbg_suffix = '.debug'
  dbg_file_path = '%s/%s%s' % (dbg_prefix_base, parts['exec_name'], dbg_suffix)

  shutil.copyfile(args.exec_path, dbg_file_path)

  # Note this needs to be in sync with debug_symbols.gni
  completion_file = '%s/.%s_dbg_success' % (args.dest, args.exec_name)
  Touch(completion_file)

  return 0


if __name__ == '__main__':
  sys.exit(main())
