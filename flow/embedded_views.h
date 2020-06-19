// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef FLUTTER_FLOW_EMBEDDED_VIEWS_H_
#define FLUTTER_FLOW_EMBEDDED_VIEWS_H_

#include <vector>

#include "flutter/flow/surface_frame.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/raster_thread_merger.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {

// TODO(chinmaygarde): Make these enum names match the style guide.
enum MutatorType { clip_rect, clip_rrect, clip_path, transform, opacity };

// Stores mutation information like clipping or transform.
//
// The `type` indicates the type of the mutation: clip_rect, transform and etc.
// Each `type` is paired with an object that supports the mutation. For example,
// if the `type` is clip_rect, `rect()` is used the represent the rect to be
// clipped. One mutation object must only contain one type of mutation.
class Mutator {
 public:
  Mutator(const Mutator& other) {
    type_ = other.type_;
    switch (other.type_) {
      case clip_rect:
        rect_ = other.rect_;
        break;
      case clip_rrect:
        rrect_ = other.rrect_;
        break;
      case clip_path:
        path_ = new SkPath(*other.path_);
        break;
      case transform:
        matrix_ = other.matrix_;
        break;
      case opacity:
        alpha_ = other.alpha_;
        break;
      default:
        break;
    }
  }

  explicit Mutator(const SkRect& rect) : type_(clip_rect), rect_(rect) {}
  explicit Mutator(const SkRRect& rrect) : type_(clip_rrect), rrect_(rrect) {}
  explicit Mutator(const SkPath& path)
      : type_(clip_path), path_(new SkPath(path)) {}
  explicit Mutator(const SkMatrix& matrix)
      : type_(transform), matrix_(matrix) {}
  explicit Mutator(const int& alpha) : type_(opacity), alpha_(alpha) {}

  const MutatorType& GetType() const { return type_; }
  const SkRect& GetRect() const { return rect_; }
  const SkRRect& GetRRect() const { return rrect_; }
  const SkPath& GetPath() const { return *path_; }
  const SkMatrix& GetMatrix() const { return matrix_; }
  const int& GetAlpha() const { return alpha_; }
  float GetAlphaFloat() const { return (alpha_ / 255.0); }

  bool operator==(const Mutator& other) const {
    if (type_ != other.type_) {
      return false;
    }
    switch (type_) {
      case clip_rect:
        return rect_ == other.rect_;
      case clip_rrect:
        return rrect_ == other.rrect_;
      case clip_path:
        return *path_ == *other.path_;
      case transform:
        return matrix_ == other.matrix_;
      case opacity:
        return alpha_ == other.alpha_;
    }

    return false;
  }

  bool operator!=(const Mutator& other) const { return !operator==(other); }

  bool IsClipType() {
    return type_ == clip_rect || type_ == clip_rrect || type_ == clip_path;
  }

  ~Mutator() {
    if (type_ == clip_path) {
      delete path_;
    }
  };

 private:
  MutatorType type_;

  union {
    SkRect rect_;
    SkRRect rrect_;
    SkMatrix matrix_;
    SkPath* path_;
    int alpha_;
  };

};  // Mutator

// A stack of mutators that can be applied to an embedded platform view.
//
// The stack may include mutators like transforms and clips, each mutator
// applies to all the mutators that are below it in the stack and to the
// embedded view.
//
// For example consider the following stack: [T1, T2, T3], where T1 is the top
// of the stack and T3 is the bottom of the stack. Applying this mutators stack
// to a platform view P1 will result in T1(T2(T2(P1))).
class MutatorsStack {
 public:
  MutatorsStack() = default;

  void PushClipRect(const SkRect& rect);
  void PushClipRRect(const SkRRect& rrect);
  void PushClipPath(const SkPath& path);
  void PushTransform(const SkMatrix& matrix);
  void PushOpacity(const int& alpha);

  // Removes the `Mutator` on the top of the stack
  // and destroys it.
  void Pop();

  // Returns an iterator pointing to the top of the stack.
  const std::vector<std::shared_ptr<Mutator>>::const_reverse_iterator Top()
      const;
  // Returns an iterator pointing to the bottom of the stack.
  const std::vector<std::shared_ptr<Mutator>>::const_reverse_iterator Bottom()
      const;
  bool is_empty() const { return vector_.empty(); }

  bool operator==(const MutatorsStack& other) const {
    if (vector_.size() != other.vector_.size()) {
      return false;
    }
    for (size_t i = 0; i < vector_.size(); i++) {
      if (*vector_[i] != *other.vector_[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator==(const std::vector<Mutator>& other) const {
    if (vector_.size() != other.size()) {
      return false;
    }
    for (size_t i = 0; i < vector_.size(); i++) {
      if (*vector_[i] != other[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const MutatorsStack& other) const {
    return !operator==(other);
  }

  bool operator!=(const std::vector<Mutator>& other) const {
    return !operator==(other);
  }

 private:
  std::vector<std::shared_ptr<Mutator>> vector_;
};  // MutatorsStack

class EmbeddedViewParams {
 public:
  EmbeddedViewParams() = default;

  EmbeddedViewParams(const EmbeddedViewParams& other) {
    offsetPixels = other.offsetPixels;
    sizePoints = other.sizePoints;
    mutatorsStack = other.mutatorsStack;
  };

  SkPoint offsetPixels;
  SkSize sizePoints;
  MutatorsStack mutatorsStack;

  bool operator==(const EmbeddedViewParams& other) const {
    return offsetPixels == other.offsetPixels &&
           sizePoints == other.sizePoints &&
           mutatorsStack == other.mutatorsStack;
  }

  // Clippings are ignored in this operation
  // TODO(cyanglaz): Factoring clipping.
  //
  SkRect GetBoundingRectAfterMutations() const {
    SkPath path;
    SkRect starting_rect = SkRect::MakeSize(sizePoints);
    path.addRect(starting_rect);
    std::vector<std::shared_ptr<Mutator>>::const_reverse_iterator iter =
        mutatorsStack.Bottom();
    while (iter != mutatorsStack.Top()) {
      switch ((*iter)->GetType()) {
        case transform: {
          const SkMatrix& matrix = (*iter)->GetMatrix();
          path.transform(matrix);
          break;
        }
        case clip_rect:
        case clip_rrect:
        case clip_path:
        case opacity:
          break;
      }
      ++iter;
    }
    return path.computeTightBounds();
  }
};

enum class PostPrerollResult { kResubmitFrame, kSuccess };

// Facilitates embedding of platform views within the flow layer tree.
//
// Used on iOS, Android (hybrid composite mode), and on embedded platforms
// that provide a system compositor as part of the project arguments.
class ExternalViewEmbedder {
  // TODO(cyanglaz): Make embedder own the `EmbeddedViewParams`.

 public:
  ExternalViewEmbedder() = default;

  virtual ~ExternalViewEmbedder() = default;

  // Usually, the root canvas is not owned by the view embedder. However, if
  // the view embedder wants to provide a canvas to the rasterizer, it may
  // return one here. This canvas takes priority over the canvas materialized
  // from the on-screen render target.
  virtual SkCanvas* GetRootCanvas() = 0;

  // Call this in-lieu of |SubmitFrame| to clear pre-roll state and
  // sets the stage for the next pre-roll.
  virtual void CancelFrame() = 0;

  virtual void BeginFrame(SkISize frame_size,
                          GrContext* context,
                          double device_pixel_ratio) = 0;

  virtual void PrerollCompositeEmbeddedView(
      int view_id,
      std::unique_ptr<EmbeddedViewParams> params) = 0;

  // This needs to get called after |Preroll| finishes on the layer tree.
  // Returns kResubmitFrame if the frame needs to be processed again, this is
  // after it does any requisite tasks needed to bring itself to a valid state.
  // Returns kSuccess if the view embedder is already in a valid state.
  virtual PostPrerollResult PostPrerollAction(
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
    return PostPrerollResult::kSuccess;
  }

  virtual std::vector<SkCanvas*> GetCurrentCanvases() = 0;

  // Must be called on the UI thread.
  virtual SkCanvas* CompositeEmbeddedView(int view_id) = 0;

  // Implementers must submit the frame by calling frame.Submit().
  //
  // This method can mutate the root Skia canvas before submitting the frame.
  //
  // It can also allocate frames for overlay surfaces to compose hybrid views.
  virtual bool SubmitFrame(GrContext* context,
                           std::unique_ptr<SurfaceFrame> frame);

  // This should only be called after |SubmitFrame|.
  // This method provides the embedder a way to do additional tasks after
  // |SubmitFrame|. After invoking this method, the current task on the
  // TaskRunner should end immediately.
  //
  // For example on the iOS embedder, threads are merged in this call.
  // A new frame on the platform thread starts immediately. If the GPU thread
  // still has some task running, there could be two frames being rendered
  // concurrently, which causes undefined behaviors.
  virtual void EndFrame(
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {}

  FML_DISALLOW_COPY_AND_ASSIGN(ExternalViewEmbedder);

};  // ExternalViewEmbedder

}  // namespace flutter

#endif  // FLUTTER_FLOW_EMBEDDED_VIEWS_H_
