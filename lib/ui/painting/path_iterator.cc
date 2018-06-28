// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/path_iterator.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_args.h"
#include "lib/tonic/dart_binding_macros.h"
#include "lib/tonic/dart_library_natives.h"

using tonic::ToDart;

namespace blink {

typedef CanvasPathIterator PathIterator;

static void PathIterator_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&CanvasPathIterator::Create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, PathIterator);

#define FOR_EACH_BINDING(V)    \
  V(PathIterator, points)      \
  V(PathIterator, verb)        \
  V(PathIterator, conicWeight) \
  V(PathIterator, next)        \
  V(PathIterator, isClosed)    \
  V(PathIterator, isCloseLine)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void CanvasPathIterator::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"PathIterator_constructor", PathIterator_constructor, 3, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

fxl::RefPtr<CanvasPathIterator> CanvasPathIterator::Create(
    const CanvasPath* path,
    bool forceClosed) {
  fxl::RefPtr<CanvasPathIterator> pathIterator =
      fxl::MakeRefCounted<CanvasPathIterator>();
  const SkPath skPath = path->path();
  pathIterator->path_iter_ =
      std::make_unique<SkPath::Iter>(skPath, forceClosed);
  pathIterator->verb_ = SkPath::kDone_Verb;

  return pathIterator;
}

CanvasPathIterator::CanvasPathIterator() {}

CanvasPathIterator::~CanvasPathIterator() {}

bool CanvasPathIterator::next(bool consumeDegenerates, bool exact) {
  verb_ = path_iter_->next(points_, consumeDegenerates, exact);
  return verb_ != SkPath::kDone_Verb;
}

tonic::Float32List CanvasPathIterator::points() {
  switch (verb_) {
    case SkPath::kMove_Verb: {
      tonic::Float32List points(
          Dart_NewTypedData(Dart_TypedData_kFloat32, 1 * 2));
      points[0] = points_[0].x();
      points[1] = points_[0].y();
      return points;
    }
    case SkPath::kLine_Verb: {
      tonic::Float32List points(
          Dart_NewTypedData(Dart_TypedData_kFloat32, 2 * 2));
      points[0] = points_[0].x();
      points[1] = points_[0].y();
      points[2] = points_[1].x();
      points[3] = points_[1].y();
      return points;
    }
    case SkPath::kConic_Verb:
      // fall-through
    case SkPath::kQuad_Verb: {
      tonic::Float32List points(
          Dart_NewTypedData(Dart_TypedData_kFloat32, 3 * 2));
      points[0] = points_[0].x();
      points[1] = points_[0].y();
      points[2] = points_[1].x();
      points[3] = points_[1].y();
      points[4] = points_[2].x();
      points[5] = points_[2].y();
      return points;
    }
    case SkPath::kCubic_Verb: {
      tonic::Float32List points(
          Dart_NewTypedData(Dart_TypedData_kFloat32, 4 * 2));
      points[0] = points_[0].x();
      points[1] = points_[0].y();
      points[2] = points_[1].x();
      points[3] = points_[1].y();
      points[4] = points_[2].x();
      points[5] = points_[2].y();
      points[6] = points_[3].x();
      points[7] = points_[3].y();
      return points;
    }
    case SkPath::kClose_Verb:
      // fall-through
    case SkPath::kDone_Verb: {
      tonic::Float32List points(Dart_NewTypedData(Dart_TypedData_kFloat32, 0));
      return points;
    }
  }
}

int CanvasPathIterator::verb() {
  return verb_;
}

float CanvasPathIterator::conicWeight() {
  return path_iter_->conicWeight();
}

bool CanvasPathIterator::isClosed() {
  return path_iter_->isClosedContour();
}

bool CanvasPathIterator::isCloseLine() {
  return path_iter_->isCloseLine();
}

}  // namespace blink
