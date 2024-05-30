// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"

using namespace Skwasm;

SKWASM_EXPORT ContourMeasureIter* contourMeasureIter_create(Path* path,
                                                            bool forceClosed,
                                                            Scalar resScale) {
  return new ContourMeasureIter(*path, forceClosed, resScale);
}

SKWASM_EXPORT ContourMeasure* contourMeasureIter_next(
    ContourMeasureIter* iter) {
  auto next = iter->next();
  if (next) {
    next->ref();
  }
  return next.get();
}

SKWASM_EXPORT void contourMeasureIter_dispose(ContourMeasureIter* iter) {
  delete iter;
}

SKWASM_EXPORT void contourMeasure_dispose(ContourMeasure* measure) {
  measure->unref();
}

SKWASM_EXPORT Scalar contourMeasure_length(ContourMeasure* measure) {
  return measure->length();
}

SKWASM_EXPORT bool contourMeasure_isClosed(ContourMeasure* measure) {
  return measure->isClosed();
}

SKWASM_EXPORT bool contourMeasure_getPosTan(ContourMeasure* measure,
                                            Scalar distance,
                                            Point* outPosition,
                                            Vector* outTangent) {
  return measure->getPosTan(distance, outPosition, outTangent);
}

SKWASM_EXPORT Path* contourMeasure_getSegment(ContourMeasure* measure,
                                              Scalar startD,
                                              Scalar stopD,
                                              bool startWithMoveTo) {
  Path* outPath = new Path();
  if (!measure->getSegment(startD, stopD, outPath, startWithMoveTo)) {
    delete outPath;
    return nullptr;
  }
  return outPath;
}
