// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"

namespace flutter {

ExternalViewEmbedder::ExternalViewEmbedder()
    : transformStack(std::make_shared<MutatorsStack>()){};

bool ExternalViewEmbedder::SubmitFrame(GrContext* context) {
  return false;
};

void MutatorsStack::pushClipRect(const SkRect& rect) {
  Mutator element = Mutator();
  element.setType(clip_rect);
  element.setRect(rect);
  vector_.push_back(element);
};

void MutatorsStack::pushClipRRect(const SkRRect& rrect) {
  Mutator element = Mutator();
  element.setType(clip_rrect);
  element.setRRect(rrect);
  vector_.push_back(element);
};

void MutatorsStack::pushTransform(const SkMatrix& matrix) {
  Mutator element = Mutator();
  element.setType(transform);
  element.setMatrix(matrix);
  vector_.push_back(element);
};

void MutatorsStack::pop() {
  vector_.pop_back();
}

std::vector<Mutator>::reverse_iterator
MutatorsStack::top() {
  return vector_.rend();
}

std::vector<Mutator>::reverse_iterator
MutatorsStack::bottom() {
  return vector_.rbegin();
}

}  // namespace flutter
