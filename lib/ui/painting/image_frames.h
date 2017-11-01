// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_ANIMATED_IMAGE_H_
#define FLUTTER_LIB_UI_PAINTING_ANIMATED_IMAGE_H_

#include "flutter/lib/ui/painting/image.h"
#include "lib/tonic/dart_wrappable.h"
#include "third_party/skia/include/core/SkImage.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

class ImageFrames : public fxl::RefCountedThreadSafe<ImageFrames>,
                          public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:

  virtual int frameCount() = 0;

  virtual fxl::RefPtr<CanvasImage> getNextFrame() = 0;

  void dispose();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

};

class SingleFrameImageFrames final : public ImageFrames {
  FRIEND_MAKE_REF_COUNTED(SingleFrameImageFrames);

 public:

  static fxl::RefPtr<SingleFrameImageFrames> Create(fxl::RefPtr<CanvasImage> frame) {
    return fxl::MakeRefCounted<SingleFrameImageFrames>(frame);
  }

  int frameCount() { return 1; }

  fxl::RefPtr<CanvasImage> getNextFrame() { return frame_; }

 private:
  SingleFrameImageFrames(fxl::RefPtr<CanvasImage> frame) { frame_ = frame; }

  fxl::RefPtr<CanvasImage> frame_;
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_PAINTING_ANIMATED_IMAGE_H_
