# Copyright 2023 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import unittest
import os
import importlib.machinery, importlib.util

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))

build_loader = importlib.machinery.SourceFileLoader(
    'build_local_engine.py', TOOLS_DIR + '/build_local_engine.py'
)
build_spec = importlib.util.spec_from_loader(build_loader.name, build_loader)
build = importlib.util.module_from_spec(build_spec)
build_loader.exec_module(build)

gn_loader = importlib.machinery.SourceFileLoader('gn', TOOLS_DIR + '/gn')
gn_spec = importlib.util.spec_from_loader(gn_loader.name, gn_loader)
gn = importlib.util.module_from_spec(gn_spec)
gn_loader.exec_module(gn)


class BuildLocalEngineTestCase(unittest.TestCase):

  def test_build_gn_args(self):

    gn_args = build.build_gn_args(0)
    self.assertTrue(gn_args.unoptimized)
    self.assertEqual(gn_args.target_os, 'android')

    gn_args = build.build_gn_args(1)
    self.assertTrue(gn_args.unoptimized)
    self.assertEqual(gn_args.target_os, 'android')
    self.assertEqual(gn_args.android_cpu, 'arm64')

    gn_args = build.build_gn_args(2)
    self.assertTrue(gn_args.unoptimized)
    self.assertEqual(gn_args.target_os, 'android')
    self.assertEqual(gn_args.android_cpu, 'x86')

    gn_args = build.build_gn_args(3)
    self.assertTrue(gn_args.unoptimized)
    self.assertEqual(gn_args.target_os, 'android')
    self.assertEqual(gn_args.android_cpu, 'x64')

    gn_args = build.build_gn_args(4)
    self.assertTrue(gn_args.unoptimized)
    self.assertEqual(gn_args.target_os, 'ios')

    gn_args = build.build_gn_args(5)
    self.assertEqual(gn_args.target_os, 'ios')
    self.assertTrue(gn_args.simulator)

    gn_args = build.build_gn_args(6)
    self.assertEqual(gn_args.target_os, 'ios')
    self.assertEqual(gn_args.simulator_cpu, 'arm64')


if __name__ == '__main__':
  unittest.main()
