#!/usr/bin/env python
# Copyright 2018 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import stat
import sys

def main():
  parser = argparse.ArgumentParser(
      description='Generate a script that runs something')
  parser.add_argument('--out',
                      help='Path to the file to generate',
                      required=True)
  parser.add_argument('--to_be_run',
                      help='The argument to `run`',
                      required=True)
  args = parser.parse_args()

  script_file = args.out
  script_path = os.path.dirname(script_file)
  if not os.path.exists(script_path):
    os.makedirs(script_path)

  script = (
    '#!/boot/bin/sh\n\n'
    'run %s\n' % args.to_be_run)
  with open(script_file, 'w') as file:
    file.write(script)
  permissions = (stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR |
                 stat.S_IRGRP | stat.S_IWGRP | stat.S_IXGRP |
                 stat.S_IROTH)
  os.chmod(script_file, permissions)


if __name__ == '__main__':
  sys.exit(main())
