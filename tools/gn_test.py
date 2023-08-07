# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import unittest

import os
import importlib.machinery, importlib.util

SKY_TOOLS = os.path.dirname(os.path.abspath(__file__))
loader = importlib.machinery.SourceFileLoader('gn', SKY_TOOLS + '/gn')
spec = importlib.util.spec_from_loader(loader.name, loader)
gn = importlib.util.module_from_spec(spec)
loader.exec_module(gn)


class GNTestCase(unittest.TestCase):

  def _expect_build_dir(self, arg_list, expected_build_dir):
    args = gn.parse_args(['gn'] + arg_list)
    self.assertEqual(gn.get_out_dir(args), expected_build_dir)

  def test_get_out_dir(self):
    self._expect_build_dir([], 'out/host_debug')
    self._expect_build_dir(['--runtime-mode', 'release'], 'out/host_release')
    self._expect_build_dir(['--ios'], 'out/ios_debug')
    self._expect_build_dir(['--ios', '--runtime-mode', 'release'],
                           'out/ios_release')
    self._expect_build_dir(['--android'], 'out/android_debug')
    self._expect_build_dir(['--android', '--runtime-mode', 'release'],
                           'out/android_release')

  def _gn_args(self, arg_list):
    args = gn.parse_args(['gn'] + arg_list)
    return gn.to_gn_args(args)

  def test_to_gn_args(self):
    # This would not necessarily be true on a 32-bit machine?
    self.assertEquals(
        self._gn_args(['--ios', '--simulator'])['target_cpu'], 'x64'
    )
    self.assertEquals(self._gn_args(['--ios'])['target_cpu'], 'arm')

  def test_defaults(self):
    parser = gn.init_parser()
    args = parser.parse_args()
    self.assertFalse(args.unoptimized)
    self.assertIsNone(args.target_os)
    self.assertEqual(args.android_cpu, 'arm')
    self.assertFalse(args.simulator)
    self.assertEqual(args.simulator_cpu, 'x64')


if __name__ == '__main__':
  unittest.main()
