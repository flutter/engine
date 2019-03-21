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

def GetFIDLFiles(sdk_path, package_json_path):
  # Read the JSON file to figure out the fidl sources.
  fidl_sources = []
  
  with open(package_json_path) as json_file:
    parsed_json = json.load(json_file)
    
    package_deps = parsed_json['deps']
    if not package_deps:
      package_deps = []

    for dep in package_deps:
      dep_json = os.path.join(sdk_path, 'fidl', dep, 'meta.json')
      assert os.path.exists(dep_json), "%s inferred from deps of %s must exist." % (dep_json, package_json_path)
      fidl_sources += GetFIDLFiles(sdk_path, dep_json)
    
    json_sources = parsed_json['sources']
    assert json_sources, "%s does not contain sources." % parsed_json

    for source in json_sources:
      fidl_source = os.path.abspath(os.path.join(sdk_path, source))
      assert os.path.exists(fidl_source)
      fidl_sources.append(fidl_source)

  return fidl_sources

def main():
  parser = argparse.ArgumentParser();

  parser.add_argument('--sdk-path', dest='sdk_path', action='store', required=True)
  parser.add_argument('--package-json-path', dest='package_json_path', action='store', required=True)
  parser.add_argument('--headers-path', dest = 'headers-path', action='store', required=True)

  args = parser.parse_args()

  assert os.path.exists(args.sdk_path), "%s must exist." % args.sdk_path
  assert os.path.exists(args.package_json_path), "%s must exist." % args.package_json_path

  fidl_name = 'hello'
  fidl_sources = GetFIDLFiles(args.sdk_path, args.package_json_path)

  # Invoke the frontend to generate the intermediates.
  intermediates_json_path = os.path.abspath(os.path.join(args.sdk_path, fidl_name + '.json'))

  subprocess.check_call([
    os.path.join(args.sdk_path, 'tools', 'fidlc'),
    '--json',
    intermediates_json_path,
    '--files'
  ] + fidl_sources)

  assert os.path.exists(intermediates_json_path)

  # Invoke the CPP backend to generate the headers.
  output_headers_path = os.path.abspath(os.path.join(args.headers_path, fidl_name, 'fuchsia', 'cpp', 'fidl'))
  subprocess.check_call([
    os.path.join(args.sdk_path, 'tools', 'fidlgen'),
    '--json',
    intermediates_json_path,
     '--generators',
     'cpp',
     '--output-base',
     output_headers_path,
  ])

  return 0

if __name__ == '__main__':
  sys.exit(main())
