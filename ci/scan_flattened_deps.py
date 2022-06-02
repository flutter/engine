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

HELP_STR = "To find complete information on this vulnerability, navigate to "
OSV_VULN_DB_URL = "https://osv.dev/vulnerability/"


def ParseDepsFile(deps_flat_file):
    deps = open(deps_flat_file, 'r')
    Lines = deps.readlines()

    headers = { 'Content-Type': 'application/x-www-form-urlencoded',}
    osv_url = 'https://api.osv.dev/v1/query'
    osv_scans = []

    # Extract commit hash, call OSV with each hash
    for line in Lines:
        dep = line.strip().split('@')
        commit_hash = line.strip().split('@')[1]
        package = dep[0].split('/')[-1].split('.')[0]
        data = {"commit" : commit_hash}
        response = requests.post(osv_url, headers=headers, data=str(data), allow_redirects=True)
        print("Scanned " + package + " at " + dep[1], end = '')
        if response.json() == {}:
            print(" and found no vulnerabilities")
        if response.json() != {} and response.json().get("vulns"):
            osv_scans.append({"vulns": response.json().get("vulns"), "package": package})
            print(" and found " + str(len(response.json().get("vulns"))) + " vulnerabilit(y/ies), adding to report")
    return osv_scans


def WriteSarif(osv_scans, manifest_file):
    if len(osv_scans) == 0:
        print('No vulnerabilities detected')
    else:
        f = open('template.sarif')
        data = json.load(f)
        for scan in osv_scans:
            package = scan["package"]
            vulns = scan["vulns"]
            for vuln in vulns:
                data['runs'][0]['tool']['driver']['rules'].append(CreateRuleEntry(vuln))
                data['runs'][0]['results'].append(CreateResultEntry(vuln, package))
        print(data)

        with open(manifest_file, 'w') as out:
            json.dump(data, out)

def CreateRuleEntry(vuln: Dict[str, Any]):
    """
    Creates a Sarif rule entry from an OSV finding.
    Vuln object follows OSV Schema and is required to have 'id' and 'modified'
    """
    f = open('rule_template.json')
    rule = json.load(f)
    rule['id'] = vuln['id']
    rule['shortDescription']['text'] = vuln['id']
    rule['fullDescription']['text'] = "Vulnerability found: " + vuln['id'] + " last modified: " + vuln['modified']
    rule['help']['text'] = HELP_STR + OSV_VULN_DB_URL + vuln['id']
    return rule

def CreateResultEntry(vuln: Dict[str, Any], package: str):
    """
    Creates a Sarif res entry from an OSV entry.
    Rule finding linked to the associated rule metadata via ruleId
    """
    f = open('result_template.json')
    result = json.load(f)
    result['ruleId'] = vuln['id']
    result['message']['text'] = package
    return result

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
    osv_scans = ParseDepsFile(args.flat_deps)
    WriteSarif(osv_scans, args.output)
    return 0


if __name__ == '__main__':
    sys.exit(Main(sys.argv))