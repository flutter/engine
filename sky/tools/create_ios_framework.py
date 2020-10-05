#!/usr/bin/env python
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import subprocess
import shutil
import sys
import os


DSYMUTIL = os.path.join(os.path.dirname(__file__), '..', '..', '..',
                        'buildtools', 'mac-x64', 'clang', 'bin', 'dsymutil')

def main():
  parser = argparse.ArgumentParser(description='Creates Flutter.framework')

  parser.add_argument('--dst', type=str, required=True)
  parser.add_argument('--arm64-out-dir', type=str, required=True)
  parser.add_argument('--armv7-out-dir', type=str, required=True)
  parser.add_argument('--simulator-out-dir', type=str, required=True)
  parser.add_argument('--strip', action="store_true", default=False)
  parser.add_argument('--dsym', action="store_true", default=False)
  parser.add_argument('--strip-bitcode', dest='strip_bitcode', action="store_true", default=False)

  args = parser.parse_args()

  fat_framework = os.path.join(args.dst, 'Flutter.framework')
  arm64_framework = os.path.join(args.arm64_out_dir, 'Flutter.framework')
  armv7_framework = os.path.join(args.armv7_out_dir, 'Flutter.framework')
  simulator_framework = os.path.join(args.simulator_out_dir, 'Flutter.framework')

  arm64_dylib = os.path.join(arm64_framework, 'Flutter')
  armv7_dylib = os.path.join(armv7_framework, 'Flutter')
  simulator_dylib = os.path.join(simulator_framework, 'Flutter')

  if not os.path.isdir(arm64_framework):
    print('Cannot find iOS arm64 Framework at %s' % arm64_framework)
    return 1

  if not os.path.isdir(armv7_framework):
    print('Cannot find iOS armv7 Framework at %s' % armv7_framework)
    return 1

  if not os.path.isdir(simulator_framework):
    print('Cannot find iOS simulator Framework at %s' % simulator_framework)
    return 1

  if not os.path.isfile(arm64_dylib):
    print('Cannot find iOS arm64 dylib at %s' % arm64_dylib)
    return 1

  if not os.path.isfile(armv7_dylib):
    print('Cannot find iOS armv7 dylib at %s' % armv7_dylib)
    return 1

  if not os.path.isfile(simulator_dylib):
    print('Cannot find iOS simulator dylib at %s' % simulator_dylib)
    return 1

  if not os.path.isfile(DSYMUTIL):
    print('Cannot find dsymutil at %s' % DSYMUTIL)
    return 1

  shutil.rmtree(fat_framework, True)
  shutil.copytree(arm64_framework, fat_framework)

  linker_out = os.path.join(fat_framework, 'Flutter')

  subprocess.check_call([
    'lipo',
    arm64_dylib,
    armv7_dylib,
    simulator_dylib,
    '-create',
    '-output',
    linker_out
  ])

  if args.strip_bitcode:
    subprocess.check_call(['xcrun', 'bitcode_strip', '-r', linker_out, '-o', linker_out])
  else:
    info_plist = os.path.join(fat_framework, 'Info.plist')
    clang_version = subprocess.check_output(['xcrun', 'plutil', '-extract', 'ClangVersion', 'xml1', '-o', '-', info_plist])

    # Xcode 11.0
    clang_expected_version = '11.0.0'
    if clang_expected_version not in clang_version:
      # The version of clang compiling the frameworks must be
      # the same as the one in the minimum Xcode version enforced
      # by the Flutter tool, to prevent bitcode-related App Store rejections.
      # This version should only be incremented after the tool's Xcode
      # minimum has been incremented.
      # See postmortem requiem/doc/postmortem126775 for details.
      print('Clang version is too new, expected %s' % clang_expected_version)
      return 1

  if args.dsym:
    dsym_out = os.path.splitext(fat_framework)[0] + '.dSYM'
    subprocess.check_call([DSYMUTIL, '-o', dsym_out, linker_out])

  if args.strip:
    # copy unstripped
    unstripped_out = os.path.join(args.dst, 'Flutter.unstripped')
    shutil.copyfile(linker_out, unstripped_out)

    subprocess.check_call(["strip", "-x", "-S", linker_out])


if __name__ == '__main__':
  sys.exit(main())
