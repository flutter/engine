// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_STUDIO_H_
#define FLUTTER_FLOW_STUDIO_H_

#include <memory>

#include "flutter/common/graphics/gl_context_switch.h"
#include "flutter/fml/macros.h"

class GrDirectContext;

namespace impeller {
class AiksContext;
}  // namespace impeller

namespace flutter {

class Studio {
 public:
  Studio();

  virtual ~Studio();

  virtual bool IsValid() = 0;

  virtual GrDirectContext* GetContext() = 0;

  virtual std::unique_ptr<GLContextResult> MakeRenderContextCurrent();

  virtual bool ClearRenderContext();

  virtual bool AllowsDrawingWhenGpuDisabled() const;

  virtual bool EnableRasterCache() const;

  virtual impeller::AiksContext* GetAiksContext() const;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(Studio);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_STUDIO_H_
