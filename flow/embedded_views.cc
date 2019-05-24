// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"

namespace flutter {

ExternalViewEmbedder::ExternalViewEmbedder()
    : transformStack(std::make_shared<FlutterEmbededViewTransformStack>()){};

bool ExternalViewEmbedder::SubmitFrame(GrContext* context) {
  return false;
};

#pragma mark - FlutterEmbededViewTransformStack

void FlutterEmbededViewTransformStack::pushClipRect(const SkRect& rect) {
  FlutterEmbededViewTransformElement element =
      FlutterEmbededViewTransformElement();
  element.setType(clip_rect);
  element.setRect(rect);
  vector_.push_back(element);
};

void FlutterEmbededViewTransformStack::pushTransform(const SkMatrix& matrix) {
  FlutterEmbededViewTransformElement element =
      FlutterEmbededViewTransformElement();
  element.setType(transform);
  element.setMatrix(matrix);
  vector_.push_back(element);
};

void FlutterEmbededViewTransformStack::pop() {
  vector_.pop_back();
}

std::vector<FlutterEmbededViewTransformElement>::iterator
FlutterEmbededViewTransformStack::begin() {
  return vector_.begin();
}

std::vector<FlutterEmbededViewTransformElement>::iterator
FlutterEmbededViewTransformStack::end() {
  return vector_.end();
}

}  // namespace flutter
