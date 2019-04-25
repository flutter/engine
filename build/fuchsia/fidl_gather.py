#!/usr/bin/env python
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import json
import subprocess
import os
import argparse

def GetFIDLFiles(sdk_base, path):
  with open(path) as json_file:
    parsed = json.load(json_file)
    result = []
    deps =  parsed['deps']
    for dep in deps:
      dep_meta_json = os.path.abspath('%s/fidl/%s/meta.json' % (sdk_base, dep))
      result += GetFIDLFiles(sdk_base, dep_meta_json)
    return result + parsed['sources']

def main():
  parser = argparse.ArgumentParser();

  parser.add_argument('--sdk-base', dest='sdk_base', action='store', required=True)
  parser.add_argument('--root', dest='root', action='store', required=True)
  
  args = parser.parse_args()

  for file in GetFIDLFiles(args.sdk_base, args.root):
    print file

  return 0

if __name__ == '__main__':
  sys.exit(main())
