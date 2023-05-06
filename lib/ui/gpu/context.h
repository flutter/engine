// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_GPU_H_
#define FLUTTER_LIB_UI_GPU_H_

#include <memory>

#include "impeller/context.h"

#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

/// @brief  A scene node, which may be a deserialized ipscene asset. This node
///         can be safely added as a child to multiple scene nodes, whether
///         they're in the same scene or a different scene. The deserialized
///         node itself is treated as immutable on the IO thread.
///
///         Internally, nodes may have an animation player, which is controlled
///         via the mutation log in the `DlSceneColorSource`, which is built by
///         `SceneShader`.
class GpuContext : public RefCountedDartWrappable<GpuContext> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(GpuContext);

 public:
  GpuContext(std::shared_ptr<impeller::Context> context);
  ~GpuContext() override;

  static void InitializeDefault(Dart_Handle wrapper);

 private:
  std::shared_ptr<impeller::Context> context_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_GPU_H_
