#!/usr/bin/env python3
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Usage: deps_parser.py --deps <DEPS file> --output <flattened deps>
#
# This script parses the DEPS file, extracts the fully qualified dependencies
# and writes the to a file. This file will be later used to validate the dependencies
# are pinned to a hash.

import argparse
from gzip import READ
import os
import sys
import re

SCRIPT_DIR = os.path.dirname(sys.argv[0])
CHECKOUT_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))

CHROMIUM_README_FILE = 'third_party/accessibility/README.md'
CHROMIUM_README_COMMIT_LINE = 4 # the fifth line will always contain the commit hash
CHROMIUM_PKG_NAME = 'chromium'

# Used in parsing the DEPS file.
class VarImpl(object):
    _env_vars = {
        "host_cpu": "x64",
        "host_os": "linux",
    }

    def __init__(self, local_scope):
        self._local_scope = local_scope

    def Lookup(self, var_name):
        """Implements the Var syntax."""
        if var_name in self._local_scope.get("vars", {}):
            return self._local_scope["vars"][var_name]
        # Inject default values for env variables
        if var_name in self._env_vars:
            return self._env_vars[var_name]
        raise Exception("Var is not defined: %s" % var_name)


def ParseDepsFile(deps_file):
    local_scope = {}
    var = VarImpl(local_scope)
    global_scope = {
        'Var': var.Lookup,
        'deps_os': {},
    }
    # Read the content.
    with open(deps_file, 'r') as fp:
        deps_content = fp.read()

    # Eval the content.
    exec (deps_content, global_scope, local_scope)

    # Extract the deps and filter.
    deps = local_scope.get('deps', {})
    filtered_deps = []
    for k, v in deps.items():
        # We currently do not support packages or cipd which are represented
        # as dictionaries.
        if isinstance(v, str):
            print(v)
            filtered_deps.append(v)

    return filtered_deps

def ParseReadme():
    """
    Opens the Flutter Accessibility Library README and uses the commit hash
    found in the README to check for viulnerabilities.
    The commit hash in this README will always be in the same format
    """
    file_path = os.path.join(CHECKOUT_ROOT, CHROMIUM_README_FILE)
    file = open(file_path)
    # read the content of the file opened
    content = file.readlines()
    commit_line = content[CHROMIUM_README_COMMIT_LINE]
    print("commit line: " + commit_line)
    commit = re.search(r"(?<=\[).*(?=\])", commit_line)
    return CHROMIUM_PKG_NAME + "@" + commit.group()




def WriteManifest(deps, manifest_file):
    print('\n'.join(sorted(deps)))
    with open(manifest_file, 'w') as manifest:
        manifest.write('\n'.join(sorted(deps)))

def ParseArgs(args):
    args = args[1:]
    parser = argparse.ArgumentParser(
        description='A script to flatten a gclient DEPS file.')

    parser.add_argument(
        '--deps',
        '-d',
        type=str,
        help='Input DEPS file.',
        default=os.path.join(CHECKOUT_ROOT, 'DEPS'))
    parser.add_argument(
        '--output',
        '-o',
        type=str,
        help='Output flattened deps file.',
        default=os.path.join(CHECKOUT_ROOT, 'deps_flatten.txt'))

    return parser.parse_args(args)


def Main(argv):
    args = ParseArgs(argv)
    deps = ParseDepsFile(args.deps)
    deps.append(ParseReadme())
    WriteManifest(deps, args.output)
    return 0


if __name__ == '__main__':
    sys.exit(Main(sys.argv))
