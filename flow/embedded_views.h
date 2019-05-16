// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef FLUTTER_FLOW_EMBEDDED_VIEWS_H_
#define FLUTTER_FLOW_EMBEDDED_VIEWS_H_

#include <vector>

#include "flutter/fml/memory/ref_counted.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRect.h"

namespace flutter {

class EmbeddedViewParams {
 public:
  SkPoint offsetPixels;
  SkSize sizePoints;
};

class FlutterEmbededViewTransformStack;
class FlutterEmbededViewTransformElement;

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
  void clipRect(const SkRect& rect);
  void clipRRect(const SkRRect& rect);

  void popTransform();
  std::vector<FlutterEmbededViewTransformElement>::iterator getTransformStackIterator();
private:
  std::unique_ptr<FlutterEmbededViewTransformStack> transfromStack_;
}; // ExternalViewEmbedder

class FlutterEmbededViewTransformStack {

public:
  void pushClipRect(const SkRect& rect);
  void pushClipRRect(const SkRRect& rect);
  void pushClipPath(const SkPath& rect);

  // Removes the `FlutterEmbededViewTransformElement` on the top of the stack and destroys it.
  void pop();

  // Returns the iterator points to the bottom of the stack.
  std::vector<FlutterEmbededViewTransformElement>::iterator begin();
private:

  std::vector<FlutterEmbededViewTransformElement> vector_;
}; //FlutterEmbededViewTransformStack

enum FlutterEmbededViewTransformType {clip_rect, clip_rrect};

class FlutterEmbededViewTransformElement {
public:

  void setType(const FlutterEmbededViewTransformType type) {type_ = type;}
  void setRect(const SkRect &rect){rect_ = rect;}

  FlutterEmbededViewTransformType type() {return type_;}
  SkRect rect() {return rect_;}
  SkRRect rrect() {return rrect_;}

private:

  FlutterEmbededViewTransformType type_;
  SkRect rect_;
  SkRRect rrect_;
}; // FlutterEmbededViewTransformElement

}  // namespace flutter

#endif  // FLUTTER_FLOW_EMBEDDED_VIEWS_H_
