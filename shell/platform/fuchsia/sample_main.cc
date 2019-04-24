// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/zx/vmo.h>

int main(int argc, char const* argv[]) {
  zx::vmo vmo;
  zx::vmo::create(0, 0, &vmo);
  return 0;
}
