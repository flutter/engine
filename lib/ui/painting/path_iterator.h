// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_PATH_ITERATOR_H_
#define FLUTTER_LIB_UI_PAINTING_PATH_ITERATOR_H_

#include "flutter/lib/ui/painting/path.h"
#include "lib/tonic/dart_wrappable.h"
#include "lib/tonic/typed_data/float64_list.h"
#include "third_party/skia/include/core/SkPath.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

class CanvasPathIterator : public fxl::RefCountedThreadSafe<CanvasPathIterator>,
                   public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();
  FRIEND_MAKE_REF_COUNTED(CanvasPathIterator);

 public:
  ~CanvasPathIterator() override;
  static fxl::RefPtr<CanvasPathIterator> Create(const CanvasPath* path, bool forceClosed);

  bool next(bool consumeDegenerates = true, bool exact = false);
  int verb();
  tonic::Float32List points();
  float conicWeight();
  bool isCloseLine();
  bool isClosed();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

  const SkPath::Iter& pathIter() const { return *path_iter_; }

 private:
  CanvasPathIterator();

  std::unique_ptr<SkPath::Iter> path_iter_;
  SkPath::Verb verb_;
  SkPoint points_[4];
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_PAINTING_PATH_ITERATOR_H_
