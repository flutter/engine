// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_REGION_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_REGION_H_

#include "third_party/skia/include/core/SkRect.h"

#include <memory>
#include <vector>

namespace flutter {

typedef std::uint32_t SpanChunkHandle;
class SpanBuffer;

/// Represents a region as a collection of non-overlapping rectangles.
/// Implements a subset of SkRegion functionality optimized for quickly
/// converting set of overlapping rectangles to non-overlapping rectangles.
class DlRegion {
 public:
  /// Creates empty region.
  DlRegion();

  /// Creates region by bulk adding the rectangles.
  /// Matches SkRegion::op(rect, SkRegion::kUnion_Op) behavior.
  explicit DlRegion(const std::vector<SkIRect>& rects);

  /// Creates copy of the region.
  DlRegion(const DlRegion& r) : flutter::DlRegion(r, false) {}

  /// Creates copy of the region.
  /// If |share_buffer| is true, the new region will share the same internal
  /// span buffer as the original region. This means that both region must
  /// be used on the same thread.
  DlRegion(const DlRegion&, bool share_buffer);

  ~DlRegion();

  /// Adds another region to this region.
  /// Matches SkRegion::op(rect, SkRegion::kUnion_Op) behavior.
  void addRegion(const DlRegion& region);

  /// Returns list of non-overlapping rectangles that cover current region.
  /// If |deband| is false, each span line will result in separate rectangles,
  /// closely matching SkRegion::Iterator behavior.
  /// If |deband| is true, matching rectangles from adjacent span lines will be
  /// merged into single rectange.
  std::vector<SkIRect> getRects(bool deband = true) const;

  /// Returns whether this region intersects with a rectangle.
  bool intersects(const SkIRect& rect) const;

  /// Returns whether this region intersects with another region.
  bool intersects(const DlRegion& region) const;

  /// Returns maximum and minimum axis values of rectangles in this region.
  /// If region is empty returns SKIRect::MakeEmpty().
  const SkIRect& bounds() const { return bounds_; }

 private:
  void addRects(const std::vector<SkIRect>& rects);

  friend class SpanBuffer;
  struct Span {
    int32_t left;
    int32_t right;
  };
  typedef std::vector<Span> SpanVec;
  struct SpanLine {
    int32_t top;
    int32_t bottom;
    SpanChunkHandle chunk_handle;

    void insertSpan(SpanBuffer& span_buffer, int32_t left, int32_t right);
    void insertSpans(SpanBuffer& span_buffer,
                     const Span* begin,
                     const Span* end);
    bool spansEqual(SpanBuffer& span_buffer, const SpanLine& l2) const;
    bool spansEqual(SpanBuffer& span_buffer, const SpanVec& vec) const;
  };

  typedef std::vector<SpanLine> LineVec;

  bool isEmpty() const { return lines_.empty(); }
  bool isComplex() const;

  std::vector<SpanLine> lines_;

  SkIRect bounds_ = SkIRect::MakeEmpty();

  void insertLine(size_t position, SpanLine line);
  LineVec::iterator removeLine(LineVec::iterator position);

  SpanLine makeLine(int32_t top,
                    int32_t bottom,
                    int32_t spanLeft,
                    int32_t spanRight);
  SpanLine makeLine(int32_t top, int32_t bottom, const SpanVec&);
  SpanLine duplicateLine(int32_t top, int32_t bottom, SpanChunkHandle handle);
  SpanLine duplicateLine(int32_t top,
                         int32_t bottom,
                         SpanBuffer* span_buffer,
                         SpanChunkHandle handle);
  SpanLine mergeLines(int32_t top,
                      int32_t bottom,
                      SpanChunkHandle our_handle,
                      SpanBuffer* their_span_buffer,
                      SpanChunkHandle their_handle);

  bool spansIntersect(const Span* begin1,
                      const Span* end1,
                      const Span* begin2,
                      const Span* end2) const;

  std::shared_ptr<SpanBuffer> span_buffer_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_REGION_H_
