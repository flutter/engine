// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"
#include "wrappers.h"

using namespace Skwasm;

RTreeFactory bbhFactory;

SKWASM_EXPORT PictureRecorder* pictureRecorder_create() {
  return new PictureRecorder();
}

SKWASM_EXPORT void pictureRecorder_dispose(PictureRecorder* recorder) {
  delete recorder;
}

SKWASM_EXPORT Canvas* pictureRecorder_beginRecording(PictureRecorder* recorder,
                                                     const Rect* cullRect) {
  return recorder->beginRecording(*cullRect, &bbhFactory);
}

SKWASM_EXPORT Picture* pictureRecorder_endRecording(PictureRecorder* recorder) {
  return recorder->finishRecordingAsPicture().release();
}

SKWASM_EXPORT void picture_getCullRect(Picture* picture, Rect* outRect) {
  *outRect = picture->cullRect();
}

SKWASM_EXPORT void picture_dispose(Picture* picture) {
  picture->unref();
}

SKWASM_EXPORT uint32_t picture_approximateBytesUsed(Picture* picture) {
  return static_cast<uint32_t>(picture->approximateBytesUsed());
}
