// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_RESOURCE_CONTEXT_H_
#define FLUTTER_LIB_UI_PAINTING_RESOURCE_CONTEXT_H_

#include "lib/fxl/macros.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace blink {

class ResourceContext {
 public:
  static void Set(GrContext* context);
  static ResourceContext* Acquire();
  static void Freeze();
  static void Unfreeze();

  ResourceContext();
  ~ResourceContext();
  GrContext* Get();

  FXL_DISALLOW_COPY_AND_ASSIGN(ResourceContext);
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_PAINTING_RESOURCE_CONTEXT_H_
