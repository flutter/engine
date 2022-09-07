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
import time

SCRIPT_DIR = os.path.dirname(sys.argv[0])
CHECKOUT_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))

HELP_STR = "To find complete information on this vulnerability, navigate to "
# TODO -- use prefix matching for this rather than always to OSV
OSV_VULN_DB_URL = "https://osv.dev/vulnerability/"
DEPS = os.path.join(CHECKOUT_ROOT, 'DEPS')

failed_deps = [] # deps which fail to be be cloned or git-merge based
old_deps = [] # deps which have not been updated in more than 1 year

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

    os.mkdir('clone-test') #clone deps with upstream into temporary dir

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
    print("Dependencies that have not been rolled in at least 1 year: " + ', '.join(old_deps))

    # Query OSV API using common ancestor commit for each dep
    # return any vulnerabilities found
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
            print(' '.join(filtered_results))
            return filtered_results
    return {}

def getCommonAncestorCommit(dep):
    """
    Given an input of a mirrored dep,
    compare to the mapping of deps to their upstream
    in DEPS and find a common ancestor
    commit SHA value.

    This is done by first cloning the mirrored dep,
    then a branch which tracks the upstream.
    From there,  git merge-base operates using the HEAD
    commit SHA of the upstream branch and the pinned
    SHA value of the mirrored branch
    """
    # dep[0] contains the mirror repo
    # dep[1] contains the mirror's pinned SHA
    # upstream is the origin repo
    dep_name = dep[0].split('/')[-1].split('.')[0]
    with open(DEPS,'r', encoding='utf-8') as f:
      local_scope = {}
      global_scope = {'Var': lambda x: x} # dummy lambda
      # Read the content.
      with open(DEPS, 'r') as f:
          deps_content = f.read()

      # Eval the content.
      exec(deps_content, global_scope, local_scope)

      # Extract the deps and filter.
      deps = local_scope.get('vars').get('upstream_urls')

      if(dep_name in deps):
        try:
          # get the upstream URL from the mapping in DEPS file
          upstream = deps.get(dep_name)
          # clone dependency from mirror
          os.chdir('./clone-test')
          print(f'attempting: git clone --quiet {dep[0]}')
          os.system(f'git clone {dep[0]} --quiet {dep_name}')
          os.chdir(f'./{dep_name}')

          # check how old pinned commit is
          dep_roll_date = subprocess.check_output(f'git show -s --format=%ct {dep[1]}', shell=True).decode()
          print("dep roll date is " + dep_roll_date)
          years = (time.time() - int(dep_roll_date)) / 31556952 # number converts to years TODO - replace with more elegant than raw number
          if years >= 1:
            print(f'Old dep found: {dep[0]} is from {dep_roll_date}')
            old_deps.append(dep[0])

          # create branch that will track the upstream dep
          print('attempting to add upstream remote from: ' + upstream)
          os.system(f'git remote add upstream {upstream}')
          os.system(f'git fetch --quiet upstream')

          # get name of default branch for upstream
          default_branch = subprocess.check_output(f'git remote show upstream | sed -n \'/HEAD branch/s/.*: //p\'', shell=True).decode()
          print("default_branch found: " + default_branch)

          # make upstream branch track the upstream dep
          os.system(f'git checkout -b upstream --track upstream/{default_branch}')

          # get the most recent commit from defaul branch of upstream
          commit = subprocess.check_output("git for-each-ref --format='%(objectname:short)' refs/heads/upstream", shell=True)
          commit = commit.decode().strip()
          print("commit found: " + commit)
          print(f'git merge-base {commit} {dep[1]}')

          # perform merge-base on most recent default branch commit and pinned mirror commit
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