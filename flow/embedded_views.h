// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef FLUTTER_FLOW_EMBEDDED_VIEWS_H_
#define FLUTTER_FLOW_EMBEDDED_VIEWS_H_

#include <vector>

#include "flutter/fml/memory/ref_counted.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

class EmbeddedViewMutatorStack;
class EmbeddedViewMutator;

class EmbeddedViewParams {
 public:
  SkPoint offsetPixels;
  SkSize sizePoints;
  std::shared_ptr<EmbeddedViewMutatorStack> transformStack;

  bool operator==(const EmbeddedViewParams& other) const {
    return offsetPixels == other.offsetPixels &&
           sizePoints == other.sizePoints &&
           transformStack == other.transformStack;
  }
};

// This is only used on iOS when running in a non headless mode,
// in this case ExternalViewEmbedder is a reference to the
// FlutterPlatformViewsController which is owned by FlutterViewController.
class ExternalViewEmbedder {
 public:
  ExternalViewEmbedder();

  virtual void BeginFrame(SkISize frame_size) = 0;

  virtual void PrerollCompositeEmbeddedView(int view_id) = 0;

  virtual std::vector<SkCanvas*> GetCurrentCanvases() = 0;

  // Must be called on the UI thread.
  virtual SkCanvas* CompositeEmbeddedView(int view_id,
                                          const EmbeddedViewParams& params) = 0;

  virtual bool SubmitFrame(GrContext* context);

  virtual ~ExternalViewEmbedder() = default;

  FML_DISALLOW_COPY_AND_ASSIGN(ExternalViewEmbedder);

  std::shared_ptr<EmbeddedViewMutatorStack> transformStack;

};  // ExternalViewEmbedder

class EmbeddedViewMutatorStack {
 public:
  void pushClipRect(const SkRect& rect);
  void pushClipRRect(const SkRRect& rrect);
  void pushClipPath(const SkPath& path);

  void pushTransform(const SkMatrix& matrix);

  // Removes the `EmbeddedViewMutator` on the top of the stack
  // and destroys it.
  void pop();

  // Returns the iterator points to the bottom of the stack.
  // When we composite a embedded view, this is the first mutator we should
  // apply to the view. And we should iterate through all the mutators until we
  // reach `rend()` and apply all the mutations to the view along the way.
  std::vector<EmbeddedViewMutator>::reverse_iterator rbegin();
  // Returns the iterator points to the top of the stack.
  // When we composite a embedded view, this is the last mutator we apply to the
  // view.
  std::vector<EmbeddedViewMutator>::reverse_iterator rend();

  bool operator==(const EmbeddedViewMutatorStack& other) const {
    return vector_ == other.vector_;
  }

 private:
  std::vector<EmbeddedViewMutator> vector_;
};  // EmbeddedViewMutatorStack

enum EmbeddedViewMutationType { clip_rect, clip_rrect, clip_path, transform };

class EmbeddedViewMutator {
 public:
  void setType(const EmbeddedViewMutationType type) { type_ = type; }
  void setRect(const SkRect& rect) { rect_ = rect; }
  void setRRect(const SkRRect& rrect) { rrect_ = rrect; }
  void setMatrix(const SkMatrix& matrix) { matrix_ = matrix; }

  EmbeddedViewMutationType type() { return type_; }
  SkRect rect() { return rect_; }
  SkRRect rrect() { return rrect_; }
  SkPath path() { return path_; }
  SkMatrix matrix() { return matrix_; }

  bool operator==(const EmbeddedViewMutator& other) const {
    if (type_ != other.type_) {
      return false;
    }
    if (type_ == clip_rect && rect_ == other.rect_) {
      return true;
    }
    if (type_ == clip_rect && rrect_ == other.rrect_) {
      return true;
    }
    if (type_ == clip_path && path_ == other.path_) {
      return true;
    }
    if (type_ == transform && matrix_ == other.matrix_) {
      return true;
    }
    return false;
  }

  bool isClipType() {
    return type_ == clip_rect || type_ == clip_rrect || type_ == clip_path;
  }

 private:
  EmbeddedViewMutationType type_;
  SkRect rect_;
  SkRRect rrect_;
  SkPath path_;
  SkMatrix matrix_;
};  // EmbeddedViewMutator

}  // namespace flutter

#endif  // FLUTTER_FLOW_EMBEDDED_VIEWS_H_
