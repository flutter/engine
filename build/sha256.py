#!/usr/bin/env python3
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import hashlib
import sys


def main(args):
  with open(args.source, 'rb') as source_file:
    digest = hashlib.file_digest(source_file, "sha256")

  with open(args.output, 'w') as output_file:
    output_file.write(digest.hexdigest())


if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Write the SHA-256 hex digest of a file.')
  parser.add_argument('--source', help='Path to file to hash', type=str, required=True)
  parser.add_argument(
      '--output', help='Path to file to output the SHA-256 hex digest', type=str, required=True
  )
  sys.exit(main(parser.parse_args()))
