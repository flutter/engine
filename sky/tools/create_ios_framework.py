#!/usr/bin/env python3
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Generates and zip the ios flutter framework including the architecture
# dependent snapshot.

import argparse
import os
import sys

from create_xcframework import create_xcframework  # pylint: disable=import-error
from utils import (  # pylint: disable=import-error
    assert_directory, assert_file, buildroot_relative_path, copy_binary, copy_tree, create_zip,
    extract_dsym, lipo, strip_binary, write_codesign_config
)


def main():
  parser = argparse.ArgumentParser(
      description=(
          'Creates Flutter.framework, Flutter.xcframework and '
          'copies architecture-dependent gen_snapshot binaries to output dir'
      )
  )

  parser.add_argument('--dst', type=str, required=True)
  parser.add_argument('--x64-out-dir', type=str)
  parser.add_argument('--arm64-out-dir', type=str, required=True)
  parser.add_argument('--simulator-x64-out-dir', type=str, required=True)
  parser.add_argument('--simulator-arm64-out-dir', type=str, required=False)
  parser.add_argument('--strip', action='store_true', default=False)
  parser.add_argument('--dsym', action='store_true', default=False)

  args = parser.parse_args()

  dst = (args.dst if os.path.isabs(args.dst) else buildroot_relative_path(args.dst))

  arm64_out_dir = (
      args.arm64_out_dir
      if os.path.isabs(args.arm64_out_dir) else buildroot_relative_path(args.arm64_out_dir)
  )

  x64_out_dir = None
  if args.x64_out_dir:
    x64_out_dir = (
        args.x64_out_dir
        if os.path.isabs(args.x64_out_dir) else buildroot_relative_path(args.x64_out_dir)
    )

  simulator_x64_out_dir = None
  if args.simulator_x64_out_dir:
    simulator_x64_out_dir = (
        args.simulator_x64_out_dir if os.path.isabs(args.simulator_x64_out_dir) else
        buildroot_relative_path(args.simulator_x64_out_dir)
    )

  framework = os.path.join(dst, 'Flutter.framework')
  simulator_framework = os.path.join(dst, 'sim', 'Flutter.framework')
  arm64_framework = os.path.join(arm64_out_dir, 'Flutter.framework')
  simulator_x64_framework = os.path.join(simulator_x64_out_dir, 'Flutter.framework')

  simulator_arm64_out_dir = None
  if args.simulator_arm64_out_dir:
    simulator_arm64_out_dir = (
        args.simulator_arm64_out_dir if os.path.isabs(args.simulator_arm64_out_dir) else
        buildroot_relative_path(args.simulator_arm64_out_dir)
    )

  if args.simulator_arm64_out_dir is not None:
    simulator_arm64_framework = os.path.join(simulator_arm64_out_dir, 'Flutter.framework')

  assert_directory(arm64_framework, 'iOS arm64 framework')
  assert_directory(simulator_arm64_framework, 'iOS arm64 simulator framework')
  assert_directory(simulator_x64_framework, 'iOS x64 simulator framework')
  create_framework(
      args, dst, framework, arm64_framework, simulator_framework, simulator_x64_framework,
      simulator_arm64_framework
  )

  extension_safe_dst = os.path.join(dst, 'extension_safe')
  create_extension_safe_framework(
      args, extension_safe_dst, '%s_extension_safe' % arm64_out_dir,
      '%s_extension_safe' % simulator_x64_out_dir, '%s_extension_safe' % simulator_arm64_out_dir
  )

  # Copy gen_snapshot binary to destination directory.
  if arm64_out_dir:
    gen_snapshot = os.path.join(arm64_out_dir, 'gen_snapshot_arm64')
    copy_binary(gen_snapshot, os.path.join(dst, 'gen_snapshot_arm64'))
  if x64_out_dir:
    gen_snapshot = os.path.join(x64_out_dir, 'gen_snapshot_x64')
    copy_binary(gen_snapshot, os.path.join(dst, 'gen_snapshot_x64'))

  zip_archive(dst)
  return 0


def create_extension_safe_framework( # pylint: disable=too-many-arguments
    args, dst, arm64_out_dir, simulator_x64_out_dir, simulator_arm64_out_dir
):
  framework = os.path.join(dst, 'Flutter.framework')
  simulator_framework = os.path.join(dst, 'sim', 'Flutter.framework')
  arm64_framework = os.path.join(arm64_out_dir, 'Flutter.framework')
  simulator_x64_framework = os.path.join(simulator_x64_out_dir, 'Flutter.framework')
  simulator_arm64_framework = os.path.join(simulator_arm64_out_dir, 'Flutter.framework')

  if not os.path.isdir(arm64_framework):
    print('Cannot find extension safe iOS arm64 Framework at %s' % arm64_framework)
    return 1

  if not os.path.isdir(simulator_x64_framework):
    print('Cannot find extension safe iOS x64 simulator Framework at %s' % simulator_x64_framework)
    return 1

  create_framework(
      args, dst, framework, arm64_framework, simulator_framework, simulator_x64_framework,
      simulator_arm64_framework
  )
  return 0


def create_framework(  # pylint: disable=too-many-arguments
    args, dst, framework, arm64_framework, simulator_framework,
    simulator_x64_framework, simulator_arm64_framework
):
  arm64_dylib = os.path.join(arm64_framework, 'Flutter')
  simulator_x64_dylib = os.path.join(simulator_x64_framework, 'Flutter')
  simulator_arm64_dylib = os.path.join(simulator_arm64_framework, 'Flutter')
  assert_file(arm64_dylib, 'iOS arm64 dylib')
  assert_file(simulator_arm64_dylib, 'iOS simulator arm64 dylib')
  assert_file(simulator_x64_dylib, 'iOS simulator x64 dylib')

  # Compute dsym output paths, if enabled.
  framework_dsym = None
  simulator_dsym = None
  if args.dsym:
    framework_dsym = framework + '.dSYM'
    simulator_dsym = simulator_framework + '.dSYM'

  # Emit the framework for physical devices.
  copy_tree(arm64_framework, framework)
  framework_binary = os.path.join(framework, 'Flutter')
  process_framework(args, dst, framework_binary, framework_dsym)

  # Emit the framework for simulators.
  if args.simulator_arm64_out_dir is not None:
    copy_tree(simulator_arm64_framework, simulator_framework)
    simulator_framework_binary = os.path.join(simulator_framework, 'Flutter')
    lipo([simulator_x64_dylib, simulator_arm64_dylib], simulator_framework_binary)
    process_framework(args, dst, simulator_framework_binary, simulator_dsym)
  else:
    simulator_framework = simulator_x64_framework

  # Create XCFramework from the arm-only fat framework and the arm64/x64
  # simulator frameworks, or just the x64 simulator framework if only that one
  # exists.
  xcframeworks = [simulator_framework, framework]
  dsyms = [simulator_dsym, framework_dsym] if args.dsym else None
  create_xcframework(location=dst, name='Flutter', frameworks=xcframeworks, dsyms=dsyms)

  lipo([arm64_dylib, simulator_x64_dylib], framework_binary)
  process_framework(args, dst, framework_binary, framework_dsym)
  return 0


def zip_archive(dst):
  write_codesign_config(os.path.join(dst, 'entitlements.txt'), ['gen_snapshot_arm64'])

  write_codesign_config(
      os.path.join(dst, 'without_entitlements.txt'), [
          'Flutter.xcframework/ios-arm64/Flutter.framework/Flutter',
          'Flutter.xcframework/ios-arm64_x86_64-simulator/Flutter.framework/Flutter',
          'extension_safe/Flutter.xcframework/ios-arm64/Flutter.framework/Flutter',
          'extension_safe/Flutter.xcframework/ios-arm64_x86_64-simulator/Flutter.framework/Flutter'
      ]
  )

  create_zip(
      dst, 'artifacts.zip', [
          'gen_snapshot_arm64',
          'Flutter.xcframework',
          'entitlements.txt',
          'without_entitlements.txt',
          'extension_safe/Flutter.xcframework',
      ]
  )


def process_framework(args, dst, framework_binary, dsym):
  if dsym:
    extract_dsym(framework_binary, dsym)

  if args.strip:
    unstripped_out = os.path.join(dst, 'Flutter.unstripped')
    strip_binary(framework_binary, unstripped_copy_path=unstripped_out)


if __name__ == '__main__':
  sys.exit(main())
