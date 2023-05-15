#!/usr/bin/env python3
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import subprocess
import shutil
import sys
import os

from create_xcframework import create_xcframework  # pylint: disable=import-error

buildroot_dir = os.path.abspath(
    os.path.join(os.path.realpath(__file__), '..', '..', '..', '..')
)

DSYMUTIL = os.path.join(
    os.path.dirname(__file__), '..', '..', '..', 'buildtools', 'mac-x64',
    'clang', 'bin', 'dsymutil'
)

out_dir = os.path.join(buildroot_dir, 'out')


def main():
  parser = argparse.ArgumentParser(
      description=(
          'Creates Flutter.framework, Flutter.xcframework, '
          'FlutterMacOS.framework, and FlutterMacOS.xcframework for macOS'
      )
  )

  parser.add_argument('--dst', type=str, required=True)
  parser.add_argument('--arm64-out-dir', type=str, required=True)
  parser.add_argument('--x64-out-dir', type=str, required=True)
  parser.add_argument('--strip', action='store_true', default=False)
  parser.add_argument('--dsym', action='store_true', default=False)
  # TODO(godofredoc): Remove after recipes v2 have landed.
  parser.add_argument('--zip', action='store_true', default=False)

  args = parser.parse_args()

  dst = (
      args.dst
      if os.path.isabs(args.dst) else os.path.join(buildroot_dir, args.dst)
  )

  flutter_result = generate_framework(args, dst, 'Flutter')
  if flutter_result == 1:
    return 1

  flutter_mac_result = generate_framework(args, dst, 'FlutterMacOS')
  if flutter_mac_result == 1:
    return 1

  if args.zip:
    zip_frameworks(dst)

  return 0


def generate_framework(args, dst, framework_name):
  arm64_out_dir = (
      args.arm64_out_dir if os.path.isabs(args.arm64_out_dir) else
      os.path.join(buildroot_dir, args.arm64_out_dir)
  )
  x64_out_dir = (
      args.x64_out_dir if os.path.isabs(args.x64_out_dir) else
      os.path.join(buildroot_dir, args.x64_out_dir)
  )

  fat_framework = os.path.join(dst, f'{framework_name}.framework')
  arm64_framework = os.path.join(arm64_out_dir, f'{framework_name}.framework')
  x64_framework = os.path.join(x64_out_dir, f'{framework_name}.framework')

  arm64_dylib = os.path.join(arm64_framework, framework_name)
  x64_dylib = os.path.join(x64_framework, framework_name)

  if not os.path.isdir(arm64_framework):
    print('Cannot find macOS arm64 Framework at %s' % arm64_framework)
    return 1

  if not os.path.isdir(x64_framework):
    print('Cannot find macOS x64 Framework at %s' % x64_framework)
    return 1

  if not os.path.isfile(arm64_dylib):
    print('Cannot find macOS arm64 dylib at %s' % arm64_dylib)
    return 1

  if not os.path.isfile(x64_dylib):
    print('Cannot find macOS x64 dylib at %s' % x64_dylib)
    return 1

  if not os.path.isfile(DSYMUTIL):
    print('Cannot find dsymutil at %s' % DSYMUTIL)
    return 1

  shutil.rmtree(fat_framework, True)
  shutil.copytree(arm64_framework, fat_framework, symlinks=True)
  regenerate_symlinks(framework_name, fat_framework)

  fat_framework_binary = os.path.join(
      fat_framework, 'Versions', 'A', framework_name
  )

  # Create the arm64/x64 fat framework.
  subprocess.check_call([
      'lipo', arm64_dylib, x64_dylib, '-create', '-output', fat_framework_binary
  ])

  # Create XCFramework from the arm-only fat framework.
  xcframeworks = [fat_framework]
  create_xcframework(location=dst, name=framework_name, frameworks=xcframeworks)

  process_framework(
      dst, args, framework_name, fat_framework, fat_framework_binary
  )

  return 0


def regenerate_symlinks(framework_name, fat_framework):
  """Regenerates the symlinks structure.

  Recipes V2 upload artifacts in CAS before integration and CAS follows symlinks.
  This logic regenerates the symlinks in the expected structure.
  """
  if os.path.islink(os.path.join(fat_framework, framework_name)):
    return
  os.remove(os.path.join(fat_framework, framework_name))
  shutil.rmtree(os.path.join(fat_framework, 'Headers'), True)
  shutil.rmtree(os.path.join(fat_framework, 'Modules'), True)
  shutil.rmtree(os.path.join(fat_framework, 'Resources'), True)
  current_version_path = os.path.join(fat_framework, 'Versions', 'Current')
  shutil.rmtree(current_version_path, True)
  os.symlink('A', current_version_path)
  os.symlink(
      os.path.join('Versions', 'Current', framework_name),
      os.path.join(fat_framework, framework_name)
  )
  os.symlink(
      os.path.join('Versions', 'Current', 'Headers'),
      os.path.join(fat_framework, 'Headers')
  )
  os.symlink(
      os.path.join('Versions', 'Current', 'Modules'),
      os.path.join(fat_framework, 'Modules')
  )
  os.symlink(
      os.path.join('Versions', 'Current', 'Resources'),
      os.path.join(fat_framework, 'Resources')
  )


def embed_codesign_configuration(config_path, contents):
  with open(config_path, 'w') as file:
    file.write('\n'.join(contents) + '\n')


def process_framework(
    dst, args, framework_name, fat_framework, fat_framework_binary
):
  if args.dsym:
    dsym_out = os.path.splitext(fat_framework)[0] + '.dSYM'
    subprocess.check_call([DSYMUTIL, '-o', dsym_out, fat_framework_binary])
    if args.zip:
      dsym_dst = os.path.join(dst, f'{framework_name}.dSYM')
      subprocess.check_call([
          'zip', '-r', '-y', f'{framework_name}.dSYM.zip',
          f'{framework_name}.dSYM'
      ],
                            cwd=dst)

  if args.strip:
    # copy unstripped
    unstripped_out = os.path.join(dst, f'{framework_name}.unstripped')
    shutil.copyfile(fat_framework_binary, unstripped_out)

    subprocess.check_call(['strip', '-x', '-S', fat_framework_binary])


def zip_frameworks(dst):
  filepath_with_entitlements = ['']

  filepath_without_entitlements = [
      'Flutter.xcframework/macos-arm64_x84_64/Flutter.framework/Flutter',
      'FlutterMacOS.xcframework/macos-arm64_x84_64/FlutterMacOS.framework/FlutterMacOS'
  ]

  embed_codesign_configuration(
      os.path.join(dst, 'entitlements.txt'), filepath_with_entitlements
  )

  embed_codesign_configuration(
      os.path.join(dst, 'without_entitlements.txt'),
      filepath_without_entitlements
  )

  subprocess.check_call([
      'zip', '-r', '-y', 'frameworks.zip', 'Flutter.xcframework',
      'FlutterMacOS.xcframework', 'entitlements.txt', 'without_entitlements.txt'
  ],
                        cwd=dst)


if __name__ == '__main__':
  sys.exit(main())
