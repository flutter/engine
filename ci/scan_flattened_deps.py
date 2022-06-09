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
# TODO -- use prefix matching for this rather than always to OSV
OSV_VULN_DB_URL = "https://osv.dev/vulnerability/"

sarif_log = {
  "$schema": "https://json.schemastore.org/sarif-2.1.0.json",
  "version": "2.1.0",
  "runs": [
    {
      "tool": {
        "driver": {
          "name": "OSV Scan",
          "rules": []
        }
      },
      "results": []
    }
  ]
}

sarif_result = {
    "ruleId": "N/A",
    "message": {
        "text": "OSV Scan Finding"
    },
    "locations": [{
        "physicalLocation": {
          "artifactLocation": {
            "uri": "No location associated with this finding"
          },
          "region": {
            "startLine": 1,
            "startColumn": 1,
            "endColumn": 1
          }
        }
    }]
}

sarif_rule = {
  "id": "OSV Scan",
  "name": "OSV Scan Finding",
  "shortDescription": {
    "text": "Insert OSV id"
  },
  "fullDescription": {
    "text": "Vulnerability found by scanning against the OSV API"
  },
  "help": {
    "text": "Search OSV database using ID of the vulnerability"
  },
  "defaultConfiguration": {
    "level": "error"
  },
  "properties": {
    "problem.severity": "error",
    "security-severity": "9.8",
    "tags": [
      "supply-chain",
      "dependency"
    ]
  }
}


def ParseDepsFile(deps_flat_file):
    queries = [] # list of queries to submit in bulk request to OSV API
    deps = open(deps_flat_file, 'r')
    Lines = deps.readlines()

    headers = { 'Content-Type': 'application/json',}
    osv_url = 'https://api.osv.dev/v1/querybatch'

    # Extract commit hash, save in dictionary
    for line in Lines:
        dep = line.strip().split('@')
        commit_hash = dep[1]
        queries.append({"commit" : commit_hash})

    json = {"queries": queries}
    print(json)
    responses = requests.post(osv_url, headers=headers, json=json, allow_redirects=True)
    if responses.json() == {}:
        print("Found no vulnerabilities")
    elif responses.status_code != 200:
        print("Request error")
    elif responses.json() != {} and responses.status_code==200 and responses.json().get("results"):
        results = responses.json().get("results")
        filtered_results = list(filter(lambda vuln: vuln != {}, results))
        if len(filtered_results)==0:
            print("Found no vulnerabilities")
            return {}
        else:
            print("Found " + str(len(filtered_results)) + " vulnerabilit(y/ies), adding to report")
            return filtered_results
    return {}

def WriteSarif(responses, manifest_file):
    data = sarif_log
    print("before WriteSarif: " + str(responses))
    for response in responses:
        for vuln in response['vulns']:
            data['runs'][0]['tool']['driver']['rules'].append(CreateRuleEntry(vuln))
            data['runs'][0]['results'].append(CreateResultEntry(vuln))

    with open(manifest_file, 'w') as out:
        json.dump(data, out)

def CreateRuleEntry(vuln: Dict[str, Any]):
    """
    Creates a Sarif rule entry from an OSV finding.
    Vuln object follows OSV Schema and is required to have 'id' and 'modified'
    """
    rule = sarif_rule
    rule['id'] = vuln['id']
    rule['shortDescription']['text'] = vuln['id']
    return rule

def CreateResultEntry(vuln: Dict[str, Any]):
    """
    Creates a Sarif res entry from an OSV entry.
    Rule finding linked to the associated rule metadata via ruleId
    """
    result = sarif_result
    result['ruleId'] = vuln['id']
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