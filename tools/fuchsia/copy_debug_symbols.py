#!/usr/bin/env python
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
""" Gather the build_id, prefix_dir, and exec_name given the path to executable
    also copies to the specified destination.

    The structure of debug symbols is as follows:
      .build-id/<prefix>/<exec_name>[.debug]
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time


def IsExecutable(fpath):
  return os.path.isfile(fpath) and os.access(fpath, os.X_OK)


def Which(program):
  fpath, _ = os.path.split(program)
  if fpath:
    if IsExecutable(program):
      return program
  else:
    for path in os.environ["PATH"].split(os.pathsep):
      exe_file = os.path.join(path, program)
      if IsExecutable(exe_file):
        return exe_file

  return None


def Touch(fname):
  with open(fname, 'a'):
    os.utime(fname, None)


def GetBuildIdParts(exec_path):
  if not Which('file'):
    print "'file' command is not present on PATH."
    sys.exit(1)
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
      '--executable-name',
      dest='exec_name',
      action='store',
      required=True,
      help='This is the name of the executable that we wish to layout debug symbols for.'
  )
  parser.add_argument(
      '--executable-path',
      dest='exec_path',
      action='store',
      required=True,
      help='Path to the executable on the filesystem.')
  parser.add_argument(
      '--destination-base',
      dest='dest',
      action='store',
      required=True,
      help='Path to the base directory where the debug symbols are to be laid out.'
  )
  parser.add_argument(
      '--stripped',
      dest='stripped',
      action='store_true',
      default=True,
      help='Executable at the specified path is stripped.')
  parser.add_argument(
      '--unstripped',
      dest='stripped',
      action='store_false',
      help='Executable at the specified path is unstripped.')

  args = parser.parse_args()
  assert os.path.exists(args.exec_path)
  assert os.path.exists(args.dest)

  parts = GetBuildIdParts(args.exec_path)
  dbg_prefix_base = os.path.join(args.dest, parts['prefix_dir'])

  success = False
  for _ in range(3):
    try:
      if not os.path.exists(dbg_prefix_base):
        os.makedirs(dbg_prefix_base)
      success = True
      break
    except OSError as error:
      print 'Failed to create dir %s, error: %s. sleeping...' % (
          dbg_prefix_base, error)
      time.sleep(3)

  if not success:
    print 'Unable to create directory: %s.' % dbg_prefix_base
    return 1

  dbg_suffix = ''
  if not args.stripped:
    dbg_suffix = '.debug'
  dbg_file_name = '%s%s' % (parts['exec_name'], dbg_suffix)
  dbg_file_path = os.path.join(dbg_prefix_base, dbg_file_name)

  shutil.copyfile(args.exec_path, dbg_file_path)

  # Note this needs to be in sync with debug_symbols.gni
  completion_file = os.path.join(args.dest, '%s_dbg_success' % args.exec_name)
  Touch(completion_file)

  return 0


if __name__ == '__main__':
  sys.exit(main())
