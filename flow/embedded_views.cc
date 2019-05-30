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

void FlutterEmbededViewTransformStack::pushClipRRect(const SkRRect& rrect) {
  FlutterEmbededViewTransformElement element =
      FlutterEmbededViewTransformElement();
  element.setType(clip_rrect);
  element.setRRect(rrect);
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

std::vector<FlutterEmbededViewTransformElement>::reverse_iterator
FlutterEmbededViewTransformStack::rbegin() {
  return vector_.rbegin();
}

std::vector<FlutterEmbededViewTransformElement>::reverse_iterator
FlutterEmbededViewTransformStack::rend() {
  return vector_.rend();
}

}  // namespace flutter
