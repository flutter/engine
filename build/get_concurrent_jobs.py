#!/usr/bin/env python3
# Copyright 2019 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script computes the number of concurrent jobs that can run in the
# build as a function of the machine.

import argparse
import json
import multiprocessing
import os
import re
import subprocess
import sys

UNITS = {'B': 1, 'KB': 2**10, 'MB': 2**20, 'GB': 2**30, 'TB': 2**40}


def parse_size(string):
  i = next(i for (i, c) in enumerate(string) if not c.isdigit())
  number = string[:i].strip()
  unit = string[i:].strip()
  return int(float(number) * UNITS[unit])


class ParseSize(argparse.Action):

  def __call__(self, parser, args, values, option_string=None):
    sizes = getattr(args, self.dest, [])
    for value in values:
      (k, v) = value.split('=', 1)
      sizes.append((k, parse_size(v)))
    setattr(args, self.dest, sizes)


def get_total_memory():
  if sys.platform.startswith('linux'):
    if os.path.exists("/proc/meminfo"):
      with open("/proc/meminfo") as meminfo:
        memtotal_re = re.compile(r'^MemTotal:\s*(\d*)\s*kB')
        for line in meminfo:
          match = memtotal_re.match(line)
          if match:
            return float(match.group(1)) * 2**10
  elif sys.platform == 'darwin':
    try:
      return int(subprocess.check_output(['sysctl', '-n', 'hw.memsize']))
    except Exception:
      return 0
  else:
    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
      "--memory-per-job", action=ParseSize, default=[], nargs='*')
    parser.add_argument("--reserve-memory", type=parse_size, default=0)
    args = parser.parse_args()

    total_memory = get_total_memory()

    mem_total_bytes = max(0, get_total_memory() - args.reserve_memory)
    try:
      cpu_cap = multiprocessing.cpu_count()
    except:
      cpu_cap = 1

    concurrent_jobs = {}
    for job, memory_per_job in args.memory_per_job:
      num_concurrent_jobs = int(max(1, mem_total_bytes / memory_per_job))
      concurrent_jobs[job] = min(num_concurrent_jobs, cpu_cap)

    print(json.dumps(concurrent_jobs))

    return 0


if __name__ == '__main__':
  sys.exit(main())
