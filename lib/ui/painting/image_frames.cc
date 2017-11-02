// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_frames.h"

#include "flutter/common/threads.h"
#include "flutter/lib/ui/painting/utils.h"
#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_args.h"
#include "lib/tonic/dart_binding_macros.h"
#include "lib/tonic/dart_library_natives.h"

namespace blink {

IMPLEMENT_WRAPPERTYPEINFO(ui, ImageFrames);

#define FOR_EACH_BINDING(V)    \
  V(ImageFrames, frameCount)   \
  V(ImageFrames, getNextFrame) \
  V(ImageFrames, dispose)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void ImageFrames::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

void ImageFrames::dispose() {
  ClearDartWrapper();
}

}  // namespace blink
