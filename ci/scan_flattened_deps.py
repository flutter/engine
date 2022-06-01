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
from fileinput import close
import json
import os
import sys
import requests
from typing import Any, Dict, Optional

SCRIPT_DIR = os.path.dirname(sys.argv[0])
CHECKOUT_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))


def ParseDepsFile(deps_flat_file):
    deps = open(deps_flat_file, 'r')
    Lines = deps.readlines()

    headers = { 'Content-Type': 'application/x-www-form-urlencoded',}
    osv_url = 'https://api.osv.dev/v1/query'
    vulns = []

    # Extract commit hash, call OSV with each hash
    for line in Lines:
        dep = line.strip().split('@')
        # data = {"commit" : dep[1]}
        data = {"commit" : "6879efc2c1596d11a6a6ad296f80063b558d5e0f"}
        response = requests.post(osv_url, headers=headers, data=str(data), allow_redirects=True)
        print("Scanned " + dep[0].split('/')[-1].split('.')[0] + " at " + dep[1], end = '')
        if response.json() == {}:
            print(" and found no vulnerabilities")
        if response.json() != {} and response.json().get("vulns"):
            vulns = response.json().get("vulns")
            print(" and found " + str(len(response.json().get("vulns"))) + " vulnerabilit(y/ies), adding to report")
    return vulns


def WriteSarif(vulns, manifest_file):
    if len(vulns) == 0:
        print('No vulnerabilities detected')
    else:
        f = open('template.sarif')
        data = json.load(f)
        print("vulns: " + str(vulns))
        # for vuln in vulns:
            # data['runs'][0]['tool']['driver']['rules'].append(CreateRuleEntry(vuln))
        print(data)

        with open(manifest_file, 'w') as out:
            json.dump(data, out)

def CreateRuleEntry(vuln: Dict[str, Any]):
    """
    Creates a Sarif result entry from an OSV entry.
    Vuln object follows OSV Schema and is required to have 'id' and 'modified'
    """
    f = open('rule_template.json')
    rule = json.load(f)
    rule['id'] = vuln['id']
    return rule

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
        help='Output SARIF log of vulnerabilities found in OSV database.',
        default=os.path.join(CHECKOUT_ROOT, 'osvReport.sarif'))

    return parser.parse_args(args)


def Main(argv):
    args = ParseArgs(argv)
    vulns = ParseDepsFile(args.flat_deps)
    WriteSarif(vulns, args.output)
    return 0


if __name__ == '__main__':
    sys.exit(Main(sys.argv))