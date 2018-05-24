#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil

FRONTEND_SERVER_DIR = os.getcwd()
SRC_DIR = os.path.dirname(os.path.dirname(FRONTEND_SERVER_DIR))
DART_PACKAGES_FILE = os.path.join(SRC_DIR, 'third_party', 'dart', '.packages')

with open('.package', 'w') as packages:
  with open(DART_PACKAGES_FILE, 'r') as dart_packages:
    for line in dart_packages:
      if line.startswith('#'):
        packages.write(line)
      else:
        [package, path] = line.split(':', 1)
        packages.write('%s:../../third_party/dart/%s' % (package, path))
  packages.write('flutter_kernel_transformers:../flutter_kernel_transformers')
