# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Hello!
#
# If you find that this gclient hook is failing. You are using a split
# buildroot. This is no longer supported and you must patch you .gclient file
# and update the "name" attribute to point from "src/flutter" to "src". After,
# move into to directory containing the .gclient file, remove "src" and run
# `gclient sync`. Don't forget to save any stashes, origins, and local branches.
#
# You will only have to do this once.
