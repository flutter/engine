// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/flow/studio.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"

namespace flutter_runner {

// The interface between the Flutter rasterizer and the underlying platform. May
// be constructed on any thread but will be used by the engine only on the
// raster thread.
class Studio final : public flutter::Studio {
 public:
  Studio(GrDirectContext* gr_context);

  ~Studio() override;

 private:
  GrDirectContext* gr_context_;

  // |flutter::Studio|
  bool IsValid() override;

  // |flutter::Studio|
  GrDirectContext* GetContext() override;

  FML_DISALLOW_COPY_AND_ASSIGN(Studio);
};

}  // namespace flutter_runner
