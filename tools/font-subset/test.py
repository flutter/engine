#!/usr/bin/env python
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Tests for font-subset
'''

import filecmp
import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(SCRIPT_DIR, '..', '..', '..')
MATERIAL_TTF = os.path.join(SCRIPT_DIR, 'fixtures', 'MaterialIcons-Regular.ttf')
IS_WINDOWS = sys.platform.startswith(('cygwin', 'win'))
EXE = '.exe' if IS_WINDOWS else ''
BAT = '.bat' if IS_WINDOWS else ''
AUTONINJA = 'autoninja' + BAT
FONT_SUBSET = os.path.join(SRC_DIR, 'out', 'host_debug', 'font-subset' + EXE)

COMPARE_TESTS = (
  (True,  '1.ttf', MATERIAL_TTF, [r'57347']),
  (True,  '1.ttf', MATERIAL_TTF, [r'0xE003']),
  (True,  '1.ttf', MATERIAL_TTF, [r'\uE003']),
  (False, '1.ttf', MATERIAL_TTF, [r'57348']), # False because different codepoint
  (True,  '2.ttf', MATERIAL_TTF, [r'0xE003', r'0xE004']),
  (True,  '2.ttf', MATERIAL_TTF, [r'0xE003', r'0xE004', r'57347',]), # Duplicated codepoint
  (True,  '3.ttf', MATERIAL_TTF, [r'0xE003', r'0xE004', r'0xE021',]),
)

FAIL_TESTS = [
  [FONT_SUBSET, 'output.ttf', 'does-not-exist.ttf', '1'], # non-existant input font
  [FONT_SUBSET, 'output.ttf', MATERIAL_TTF, '0xFFFFFFFF'], # Value too big.
  [FONT_SUBSET, 'output.ttf', MATERIAL_TTF, '-1'], # invalid value
  [FONT_SUBSET, 'output.ttf', MATERIAL_TTF, 'foo'], # no valid values
  [FONT_SUBSET, 'output.ttf', MATERIAL_TTF, r'0xE003', r'0x12', r'0xE004',] # codepoint not in font
]

def RunCmd(cmd, **kwargs):
  try:
    print(subprocess.check_output(cmd, **kwargs))
  except subprocess.CalledProcessError as cpe:
    print(cpe.output)
    raise cpe


def main():
  if 'GOMA_DIR' in os.environ:
    RunCmd(['python', os.path.join(os.environ['GOMA_DIR'], 'goma_ctl.py'), 'start'])
  RunCmd(['python', 'flutter/tools/gn'], cwd=SRC_DIR)
  RunCmd([AUTONINJA, '-C', 'out/host_debug', 'font-subset'], cwd=SRC_DIR)
  failures = 0
  for should_pass, golden_font, input_font, codepoints in COMPARE_TESTS:
    gen_ttf = os.path.join(SCRIPT_DIR, 'gen', golden_font)
    golden_ttf = os.path.join(SCRIPT_DIR, 'fixtures', golden_font)
    cmd = [FONT_SUBSET, gen_ttf, input_font] + codepoints
    print('Running test %s...' % cmd)
    RunCmd(cmd, cwd=SRC_DIR)
    cmp = filecmp.cmp(gen_ttf, golden_ttf, shallow=False)
    if (should_pass and not cmp) or (not should_pass and cmp):
      print('Test case %s failed.' % cmd)
      failures += 1

  devnull = open(os.devnull, 'w')
  for cmd in FAIL_TESTS:
    print('Running test %s...' % cmd)
    if subprocess.call(cmd, cwd=SRC_DIR, stdout=devnull, stderr=devnull) == 0:
      print('Command %s passed, expected failure.' % cmd)
      failures += 1
  devnull.close()

  if failures > 0:
    print('%s test(s) failed.' % failures)
    return 1

  print('All tests passed')
  return 0


if __name__ == '__main__':
  sys.exit(main())

