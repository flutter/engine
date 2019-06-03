// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"

namespace flutter {

ExternalViewEmbedder::ExternalViewEmbedder()
    : transformStack(std::make_shared<EmbeddedViewMutatorStack>()){};

bool ExternalViewEmbedder::SubmitFrame(GrContext* context) {
  return false;
};

void EmbeddedViewMutatorStack::pushClipRect(const SkRect& rect) {
  EmbeddedViewMutator element = EmbeddedViewMutator();
  element.setType(clip_rect);
  element.setRect(rect);
  vector_.push_back(element);
};

void EmbeddedViewMutatorStack::pushClipRRect(const SkRRect& rrect) {
  EmbeddedViewMutator element = EmbeddedViewMutator();
  element.setType(clip_rrect);
  element.setRRect(rrect);
  vector_.push_back(element);
};

void EmbeddedViewMutatorStack::pushTransform(const SkMatrix& matrix) {
  EmbeddedViewMutator element = EmbeddedViewMutator();
  element.setType(transform);
  element.setMatrix(matrix);
  vector_.push_back(element);
};

void EmbeddedViewMutatorStack::pop() {
  vector_.pop_back();
}

std::vector<EmbeddedViewMutator>::reverse_iterator
EmbeddedViewMutatorStack::rbegin() {
  return vector_.rbegin();
}

std::vector<EmbeddedViewMutator>::reverse_iterator
EmbeddedViewMutatorStack::rend() {
  return vector_.rend();
}

}  // namespace flutter
