#!/usr/bin/env python3
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Usage: scan_flattened_deps.py --flat-deps <flat DEPS file> --output <vulnerability report>
#
# This script parses the flattened, fully qualified dependencies, 
# and uses the OSV API to check for known vulnerabilities
# for the given hash of the dependency

import argparse
import os
import sys
import requests

SCRIPT_DIR = os.path.dirname(sys.argv[0])
CHECKOUT_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))


def ParseDepsFile(deps_flat_file):
    deps = open(deps_flat_file, 'r')
    Lines = deps.readlines()

    headers = { 'Content-Type': 'application/x-www-form-urlencoded',}
    osv_url = 'https://api.osv.dev/v1/query'


    # Extract commit hash, call OSV with each hash
    for line in Lines:
        commit_hash = line.strip().split('@')[1]
        data = {"commit" : commit_hash}
        response = requests.post('https://api.osv.dev/v1/query', headers=headers, data=str(data), allow_redirects=True)

        print(response.text)
    # return filtered_deps


# def WriteManifest(deps, manifest_file):
#     print('\n'.join(sorted(deps)))
#     with open(manifest_file, 'w') as manifest:
#         manifest.write('\n'.join(sorted(deps)))


def ParseArgs(args):
    args = args[1:]
    parser = argparse.ArgumentParser(
        description='A script to scan a flattened DEPS file using OSV API.')

    parser.add_argument(
        '--flat-deps',
        '-d',
        type=str,
        help='Input flattened DEPS file.',
        default=os.path.join(CHECKOUT_ROOT, 'deps_flatten.txt'))
    parser.add_argument(
        '--output',
        '-o',
        type=str,
        help='Output OSV report of vulnerabilities.',
        default=os.path.join(CHECKOUT_ROOT, 'osv_report.txt'))

    return parser.parse_args(args)


def Main(argv):
    args = ParseArgs(argv)
    deps = ParseDepsFile(args.flat_deps)
    # WriteManifest(deps, args.output)
    return 0


if __name__ == '__main__':
    sys.exit(Main(sys.argv))
