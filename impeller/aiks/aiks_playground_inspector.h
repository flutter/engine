// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <functional>
#include <optional>

#include "flutter/fml/macros.h"
#include "impeller/aiks/aiks_context.h"
#include "impeller/aiks/picture.h"
#include "impeller/core/capture.h"
#include "impeller/renderer/context.h"

namespace impeller {

class AiksInspector {
 public:
  AiksInspector();

  const std::optional<Picture>& RenderInspector(
      AiksContext& aiks_context,
      const std::function<std::optional<Picture>()>& picture_callback);

  // Resets (releases) the underlying |Picture| object.
  //
  // Underlying issue: <https://github.com/flutter/flutter/issues/134678>.
  //
  // The tear-down code is not running in the right order; it's torn down after
  // we shut down the |ContextVK|, which means that the Vulkan allocator still
  // has a reference to the texture referenced in |Picture|.
  //
  // TODO(matanlurey): https://github.com/flutter/flutter/issues/134748.
  void HackResetDueToTextureLeaks();

 private:
  void RenderCapture(CaptureContext& capture_context);
  void RenderCaptureElement(CaptureElement& element);

  bool capturing_ = false;
  bool wireframe_ = false;
  CaptureElement* hovered_element_ = nullptr;
  CaptureElement* selected_element_ = nullptr;
  std::optional<Picture> last_picture_;

  FML_DISALLOW_COPY_AND_ASSIGN(AiksInspector);
};

};  // namespace impeller
