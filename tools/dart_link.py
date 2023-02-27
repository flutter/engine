# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import argparse


def parse_args(args):
  args = args[1:]
  parser = argparse.ArgumentParser(
      description='A script to manage third_party/dart checkouts.'
  )
  parser.add_argument('--init', default=False, action='store_true')
  parser.add_argument('--unlink', default=False, action='store_true')
  parser.add_argument('--link', type=str)

  return parser.parse_args(args)


def main(argv):
  args = parse_args(argv)
  third_party = os.path.join(
      os.path.dirname(__file__), '..', '..', 'third_party'
  )
  dart_dir = os.path.join(third_party, 'dart')
  dart_checkout_dir = os.path.join(third_party, 'dart_checkout')

  if args.init:
    if not os.path.exists(dart_dir):
      print('Linking //third_party/dart -> //third_party/dart_checkout')
      os.symlink(dart_checkout_dir, dart_dir)
  elif args.unlink:
    print('Linking //third_party/dart -> //third_party/dart_checkout')
    if os.path.exists(dart_dir):
      os.remove(dart_dir)
    os.symlink(dart_checkout_dir, dart_dir)
  elif args.link is not None:
    if not os.path.exists(args.link):
      raise Exception(f'Directory {args.link} does not exist')
    if os.path.exists(dart_dir):
      os.remove(dart_dir)
    os.symlink(args.link, dart_dir)
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
