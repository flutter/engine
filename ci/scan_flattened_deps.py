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
import subprocess
from turtle import clone, up
import requests
from typing import Any, Dict, Optional

SCRIPT_DIR = os.path.dirname(sys.argv[0])
CHECKOUT_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))

HELP_STR = "To find complete information on this vulnerability, navigate to "
# TODO -- use prefix matching for this rather than always to OSV
OSV_VULN_DB_URL = "https://osv.dev/vulnerability/"
DEPS_UPSTREAM_MAP = os.path.join(CHECKOUT_ROOT, '3pdeps.json')

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

def sarif_result():
  """
  Returns the template for a result entry in the Sarif log,
  which is populated with CVE findings from OSV API
  """
  return {
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

def sarif_rule():
  """
  Returns the template for a rule entry in the Sarif log,
  which is populated with CVE findings from OSV API
  """
  return {
      "id": "OSV Scan",
      "name": "OSV Scan Finding",
      "shortDescription": {
        "text": "Insert OSV id"
      },
      "fullDescription": {
        "text": "Vulnerability found by scanning against the OSV API"
      },
      "help": {
        "text": "More details for this finding can be found in the OSV DB at: https://osv.dev/vulnerability/"
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
    """
    Takes input of fully qualified dependencies,
    for each dep find the common ancestor commit SHA
    from the upstream and query OSV API using that SHA

    If the commit cannot be found or the dep cannot be
    compared to an upstream, prints list of those deps
    """
    queries = [] # list of queries to submit in bulk request to OSV API
    deps = open(deps_flat_file, 'r')
    Lines = deps.readlines()

    headers = { 'Content-Type': 'application/json',}
    osv_url = 'https://api.osv.dev/v1/querybatch'

    os.system(f'mkdir clone-test') #clone deps with upstream into temporary dir

    failed_deps = []

    # Extract commit hash, save in dictionary
    for line in Lines:
        os.chdir(CHECKOUT_ROOT)
        dep = line.strip().split('@') # separate fully qualified dep into name + pinned hash 
        common_commit = getCommonAncestorCommit(dep)
        if common_commit is not None:
          queries.append({"commit" : common_commit})
        else:
          failed_deps.append(dep[0])

    print("Dependencies that could not be parsed for ancestor commits: " + ', '.join(failed_deps))
    responses = requests.post(osv_url, headers=headers, json={"queries": queries}, allow_redirects=True)
    if responses.status_code != 200:
        print("Request error")
    elif responses.json() == {}:
        print("Found no vulnerabilities")
    elif responses.json().get("results"):
        results = responses.json().get("results")
        filtered_results = list(filter(lambda vuln: vuln != {}, results))
        if len(filtered_results)>0:
            print("Found " + str(len(filtered_results)) + " vulnerabilit(y/ies), adding to report")
            return filtered_results
    return {}

def getCommonAncestorCommit(dep):
    """
    Given an input of a mirrored dep,
    compare to the mapping of deps to their upstream
    in DEPS_UPSTREAM_MAP and find a common ancestor
    commit SHA value.

    This is done by first cloning the mirrored dep,
    then a branch which tracks the upstream.
    From there,  git merge-base operates using the HEAD
    commit SHA of the upstream branch and the pinned
    SHA value of the mirrored branch
    """
    # dep[0] contains the mirror repo
    # dep[1] contains the mirror's pinned SHA
    # data[dep_name] is the origin repo
    dep_name = dep[0].split('/')[-1].split('.')[0]
    with open(DEPS_UPSTREAM_MAP,'r', encoding='utf-8') as f:
        data = json.load(f)
        if dep_name in data:
          try:
            print("chdir")
            os.chdir('./clone-test')
            print(f'attempting: git clone {dep[0]} --quiet')
            os.system(f'git clone {dep[0]} {dep_name} --quiet')
            os.chdir(f'./{dep_name}')
            os.system(f'git for-each-ref --format=\'%(refname:short) %(objectname:short)\' refs/heads')
            print('attempting to add upstream remote from: ' + data[dep_name])
            os.system(f'git remote add upstream {data[dep_name]}')
            os.system(f'git fetch upstream')
            default_branch = subprocess.check_output(f'git remote show upstream | sed -n \'/HEAD branch/s/.*: //p\'', shell=True)
            default_branch = default_branch.decode()
            print("default_branch found: " + default_branch)
            os.system(f'git checkout -b upstream --track upstream/{default_branch}')
            commit = subprocess.check_output("git for-each-ref --format='%(objectname:short)' refs/heads/upstream", shell=True)
            commit = commit.decode().strip()
            print("commit found: " + commit)
            print(f'git merge-base {commit} {dep[1]}')
            ancestorCommit = subprocess.check_output(f'git merge-base {commit} {dep[1]}', shell=True)
            ancestorCommit = ancestorCommit.decode().strip()
            print("FOUND ANCESTOR COMMIT: " + ancestorCommit)
            return ancestorCommit
          except:
            print("exception occurred")
        else:
          print("did not find dep: " + dep_name)

def WriteSarif(responses, manifest_file):
    """
    Creates a full Sarif report based on the OSV API response which
    may contain several vulnerabilities

    Combines a rule with a result in order to construct the report
    """
    data = sarif_log
    print("before WriteSarif: " + str(responses))
    for response in responses:
        for vuln in response['vulns']:
            newRule = CreateRuleEntry(vuln)
            data['runs'][0]['tool']['driver']['rules'].append(newRule)
            data['runs'][0]['results'].append(CreateResultEntry(vuln))
    with open(manifest_file, 'w') as out:
        json.dump(data, out)

def CreateRuleEntry(vuln: Dict[str, Any]):
    """
    Creates a Sarif rule entry from an OSV finding.
    Vuln object follows OSV Schema and is required to have 'id' and 'modified'
    """
    rule = sarif_rule()
    rule['id'] = vuln['id']
    rule['shortDescription']['text'] = vuln['id']
    rule['help']['text'].append("vuln['id']")
    return rule

def CreateResultEntry(vuln: Dict[str, Any]):
    """
    Creates a Sarif res entry from an OSV entry.
    Rule finding linked to the associated rule metadata via ruleId
    """
    result = sarif_result()
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