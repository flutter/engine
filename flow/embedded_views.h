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

class FlutterEmbededViewTransformStack;
class FlutterEmbededViewTransformElement;

class EmbeddedViewParams {
 public:
  SkPoint offsetPixels;
  SkSize sizePoints;

  bool operator==(const EmbeddedViewParams& other) const {
    return offsetPixels == other.offsetPixels && sizePoints == other.sizePoints && transformStack == other.transformStack;
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

#pragma mark - transforms
  std::shared_ptr<FlutterEmbededViewTransformStack> transformStack;

};  // ExternalViewEmbedder

class FlutterEmbededViewTransformStack {
 public:
  void pushClipRect(const SkRect& rect);
  void pushClipRRect(const SkRRect& rrect);
  void pushClipPath(const SkPath& path);

  void pushTransform(const SkMatrix& matrix);

  // Removes the `FlutterEmbededViewTransformElement` on the top of the stack
  // and destroys it.
  void pop();

  // Returns the iterator points to the bottom of the stack.
  std::vector<FlutterEmbededViewTransformElement>::reverse_iterator rbegin();
  // Returns the iterator points to the top of the stack.
  std::vector<FlutterEmbededViewTransformElement>::reverse_iterator rend();

  bool operator==(const FlutterEmbededViewTransformStack& other) const {
    return vector_ == other.vector_;
  }

 private:
  std::vector<FlutterEmbededViewTransformElement> vector_;
};  // FlutterEmbededViewTransformStack

enum FlutterEmbededViewTransformType {
  clip_rect,
  clip_rrect,
  clip_path,
  transform
};

class FlutterEmbededViewTransformElement {
 public:
  void setType(const FlutterEmbededViewTransformType type) { type_ = type; }
  void setRect(const SkRect& rect) { rect_ = rect; }
  void setRRect(const SkRRect& rrect) { rrect_ = rrect; }
  void setMatrix(const SkMatrix& matrix) { matrix_ = matrix; }

  FlutterEmbededViewTransformType type() { return type_; }
  SkRect rect() { return rect_; }
  SkRRect rrect() { return rrect_; }
  SkPath path() { return path_; }
  SkMatrix matrix() { return matrix_; }

  bool operator==(const FlutterEmbededViewTransformElement& other) const {
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
  FlutterEmbededViewTransformType type_;
  SkRect rect_;
  SkRRect rrect_;
  SkPath path_;
  SkMatrix matrix_;
};  // FlutterEmbededViewTransformElement

}  // namespace flutter

#endif  // FLUTTER_FLOW_EMBEDDED_VIEWS_H_
