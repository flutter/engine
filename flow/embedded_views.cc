// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"

namespace flutter {

ExternalViewEmbedder::ExternalViewEmbedder()
  :transfromStack_(std::unique_ptr<FlutterEmbededViewTransformStack>(new FlutterEmbededViewTransformStack())) {
};

bool ExternalViewEmbedder::SubmitFrame(GrContext* context) {
  return false;
};

void ExternalViewEmbedder::clipRect(const SkRect& rect) {
  transfromStack_.get()->pushClipRect(rect);
};

void ExternalViewEmbedder::popTransform() {
  transfromStack_.get()->pop();
}

std::vector<FlutterEmbededViewTransformElement>::iterator ExternalViewEmbedder::getTransformStackIterator() {
  return transfromStack_.get()->begin();
}


#pragma mark - FlutterEmbededViewTransformStack

void FlutterEmbededViewTransformStack::pushClipRect(const SkRect& rect) {
  FlutterEmbededViewTransformElement element = FlutterEmbededViewTransformElement();
  element.setType(clip_rect);
  element.setRect(rect);
  vector_.push_back(element);
};


void FlutterEmbededViewTransformStack::pop() {
  vector_.pop_back();
}

std::vector<FlutterEmbededViewTransformElement>::iterator FlutterEmbededViewTransformStack::begin() {
  return vector_.begin();
}


#pragma mark - FlutterEmbededViewTransformElement
}  // namespace flutter
