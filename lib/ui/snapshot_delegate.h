// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_SNAPSHOT_DELEGATE_H_
#define FLUTTER_LIB_UI_SNAPSHOT_DELEGATE_H_

#include <string>

#include "flutter/common/graphics/texture.h"
#include "flutter/display_list/display_list.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkPromiseImageTexture.h"
#include "third_party/skia/include/gpu/GrContextThreadSafeProxy.h"
namespace flutter {

class SnapshotDelegate {
 public:
  virtual std::pair<sk_sp<SkImage>, std::string> MakeGpuImage(
      sk_sp<DisplayList> display_list,
      SkISize picture_size) = 0;

  virtual sk_sp<SkImage> MakeRasterSnapshot(
      std::function<void(SkCanvas*)> draw_callback,
      SkISize picture_size) = 0;

  virtual sk_sp<SkImage> MakeRasterSnapshot(sk_sp<SkPicture> picture,
                                            SkISize picture_size) = 0;

  virtual sk_sp<SkImage> ConvertToRasterImage(sk_sp<SkImage> image) = 0;

  //----------------------------------------------------------------------------
  /// @brief      Gets the registry of external textures currently in use by the
  ///             rasterizer. These textures may be updated at a cadence
  ///             different from that of the Flutter application. When an
  ///             external texture is referenced in the Flutter layer tree, that
  ///             texture is composited within the Flutter layer tree.
  ///
  /// @return     A pointer to the external texture registry.
  ///
  virtual TextureRegistry* GetTextureRegistry() = 0;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_SNAPSHOT_DELEGATE_H_
