// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/embedded_views.h"

namespace flutter {

void ExternalViewEmbedder::SubmitFrame(GrDirectContext* context,
                                       std::unique_ptr<SurfaceFrame> frame) {
  frame->Submit();
};

void MutatorsStack::PushClipRect(const SkRect& rect) {
  auto element = std::make_shared<MutatorNode>(rect, Last());
  vector_.push_back(element);
};

void MutatorsStack::PushClipRRect(const SkRRect& rrect) {
  auto element = std::make_shared<MutatorNode>(rrect, Last());
  vector_.push_back(element);
};

void MutatorsStack::PushClipPath(const SkPath& path) {
  auto element = std::make_shared<MutatorNode>(path, Last());
  vector_.push_back(element);
};

void MutatorsStack::PushTransform(const SkMatrix& matrix) {
  auto element = std::make_shared<MutatorNode>(matrix, Last());
  vector_.push_back(element);
};

void MutatorsStack::PushOpacity(const int& alpha) {
  auto element = std::make_shared<MutatorNode>(alpha, Last());
  vector_.push_back(element);
};

void MutatorsStack::Pop() {
  vector_.pop_back();
};

const std::vector<std::shared_ptr<MutatorNode>>::const_reverse_iterator
MutatorsStack::Top() const {
  return vector_.rend();
};

const std::vector<std::shared_ptr<MutatorNode>>::const_reverse_iterator
MutatorsStack::Bottom() const {
  return vector_.rbegin();
};

const std::vector<std::shared_ptr<MutatorNode>>::const_iterator
MutatorsStack::Begin() const {
  return vector_.begin();
};

const std::vector<std::shared_ptr<MutatorNode>>::const_iterator
MutatorsStack::End() const {
  return vector_.end();
};

bool ExternalViewEmbedder::SupportsDynamicThreadMerging() {
  return false;
}

}  // namespace flutter
