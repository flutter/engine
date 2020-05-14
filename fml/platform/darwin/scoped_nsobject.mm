// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/darwin/scoped_nsobject.h"

namespace fml {

id scoped_nsprotocol_arc_memory_management::Retain(id object) {
  return [object retain];
}

id scoped_nsprotocol_arc_memory_management::Autorelease(id object) {
  return [object autorelease];
}

void scoped_nsprotocol_arc_memory_management::Release(id object) {
  [object release];
}

}  // namespace fml
