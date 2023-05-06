// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/gpu/context.h"

#include <memory>
#include <sstream>

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, GpuContext);

void GpuContext::InitializeDefault(Dart_Handle wrapper) {
  auto res = fml::MakeRefCounted<SceneNode>();
  res->AssociateWithDartWrapper(wrapper);
}

GpuContext::GpuContext() = default;

GpuContext::~GpuContext() = default;

}  // namespace flutter
