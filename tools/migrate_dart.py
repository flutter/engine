# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys


def main():
  third_party = os.path.join(
      os.path.dirname(__file__), '..', '..', 'third_party'
  )
  dart_dir = os.path.join(third_party, 'dart')
  if os.path.exists(dart_dir) and not os.path.islink(dart_dir):
    print(
        'Migrating //third_party/dart directory to //third_party/dart_checkout'
    )
    dart_checkout_dir = os.path.join(third_party, 'dart_checkout')
    os.rename(dart_dir, dart_checkout_dir)

  return 0


if __name__ == '__main__':
  sys.exit(main())
