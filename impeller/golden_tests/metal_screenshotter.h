// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/image/dl_image.h"
#include "flutter/fml/macros.h"
#include "flutter/impeller/aiks/picture.h"
#include "flutter/impeller/golden_tests/metal_screenshot.h"
#include "flutter/impeller/playground/playground_impl.h"

namespace impeller {
namespace testing {

/// Converts `Picture`'s and `DisplayList`'s to `MetalScreenshot`'s with the
/// playground backend.
class MetalScreenshotter {
 public:
  MetalScreenshotter();

  sk_sp<flutter::DlImage> MakeImage(const sk_sp<flutter::DisplayList>& list,
                                    const ISize& size);

  std::unique_ptr<MetalScreenshot> MakeScreenshot(
      const sk_sp<flutter::DisplayList>& list,
      const ISize& size,
      bool scale_content = true);

  std::unique_ptr<MetalScreenshot> MakeScreenshot(const Picture& picture,
                                                  const ISize& size = {300,
                                                                       300},
                                                  bool scale_content = true);

  AiksContext& GetContext() { return *aiks_context_; }

  const AiksContext& GetContext() const { return *aiks_context_; }

  const PlaygroundImpl& GetPlayground() const { return *playground_; }

 private:
  std::unique_ptr<PlaygroundImpl> playground_;
  std::unique_ptr<AiksContext> aiks_context_;
};

}  // namespace testing
}  // namespace impeller
