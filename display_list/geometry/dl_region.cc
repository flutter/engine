// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_region.h"

#include <cassert>

namespace flutter {

struct DlRegion::Span {
  int32_t left;
  int32_t right;
};

struct DlRegion::SpanLine {
  int32_t top;
  int32_t bottom;
  // For performance reasons SpanLine must be trivially constructible.
  // DlRegion is responsible for allocating and deallocating the
  // memory for spans.
  SpanVec* spans;

  /// Inserts span into this span line. If there are existing spans
  /// that overlap with the new span, they will be merged into single
  /// span.
  void insertSpan(int32_t left, int32_t right) {
    SpanVec& spans = *this->spans;

    auto size = spans.size();
    for (size_t i = 0; i < size; ++i) {
      Span& span = spans[i];
      if (right < span.left) {
        _insertSpanAt(i, left, right);
        return;
      }
      if (left > span.right) {
        continue;
      }
      size_t last_index = i;
      while (last_index + 1 < size && right >= spans[last_index + 1].left) {
        ++last_index;
      }
      span.left = std::min(span.left, left);
      span.right = std::max(spans[last_index].right, right);
      if (last_index > i) {
        spans.erase(spans.begin() + i + 1, spans.begin() + last_index + 1);
      }
      return;
    }

    spans.push_back({left, right});
  }

  bool spansEqual(const SpanLine& l2) const {
    SpanVec& spans = *this->spans;
    SpanVec& otherSpans = *l2.spans;
    assert(this != &l2);

    if (spans.size() != otherSpans.size()) {
      return false;
    }
    return memcmp(spans.data(), otherSpans.data(),
                  spans.size() * sizeof(Span)) == 0;
  }

 private:
  void _insertSpanAt(size_t index, int32_t x1, int32_t x2) {
    spans->insert(spans->begin() + index, {x1, x2});
  }
};

DlRegion::DlRegion() {
  // If we can't memmove SpanLines addRect would be signifantly slower.
  static_assert(std::is_trivially_constructible<SpanLine>::value,
                "SpanLine must be trivially constructible.");
}

DlRegion::~DlRegion() {
  for (auto& spanvec : spanvec_pool_) {
    delete spanvec;
  }
  for (auto& line : lines_) {
    delete line.spans;
  }
}

void DlRegion::addRect(const SkIRect& rect) {
  if (rect.isEmpty()) {
    return;
  }

  int32_t y1 = rect.fTop;
  int32_t y2 = rect.fBottom;

  size_t dirty_start = -1;
  size_t dirty_end = 1;

  // Marks line as dirty. Dirty lines will be checked for equality
  // later and merged as needed.
  auto mark_dirty = [&](size_t line) {
    if (dirty_start == static_cast<size_t>(-1)) {
      dirty_start = line;
      dirty_end = line;
    } else {
      dirty_start = std::min(dirty_start, line);
      dirty_end = std::max(dirty_end, line);
    }
  };

  auto upper_bound = std::upper_bound(
      lines_.begin(), lines_.end(), y1,
      [](int32_t i, const SpanLine& line) { return i < line.bottom; });

  auto start_index = upper_bound - lines_.begin();

  for (size_t i = start_index; i < lines_.size() && y1 < y2; ++i) {
    SpanLine& line = lines_[i];

    // If this is false the start index is wrong.
    assert(y1 < line.bottom);

    if (y2 <= line.top) {
      insertLine(i, makeLine(y1, y2, rect.fLeft, rect.fRight));
      mark_dirty(i);
      y1 = y2;
      break;
    }
    if (y1 < line.top) {
      auto prevLineStart = line.top;
      insertLine(i, makeLine(y1, prevLineStart, rect.fLeft, rect.fRight));
      mark_dirty(i);
      y1 = prevLineStart;
      continue;
    }
    if (y1 > line.top) {
      // duplicate line
      auto prevLineEnd = line.bottom;
      line.bottom = y1;
      mark_dirty(i);
      insertLine(i + 1, makeLine(y1, prevLineEnd, *line.spans));
      continue;
    }
    assert(y1 == line.top);
    if (y2 < line.bottom) {
      // duplicate line
      auto newLine = makeLine(y2, line.bottom, *line.spans);
      line.bottom = y2;
      line.insertSpan(rect.fLeft, rect.fRight);
      insertLine(i + 1, newLine);
      y1 = y2;
      mark_dirty(i);
      break;
    }
    assert(y2 >= line.bottom);
    line.insertSpan(rect.fLeft, rect.fRight);
    mark_dirty(i);
    y1 = line.bottom;
  }

  if (y1 < y2) {
    lines_.push_back(makeLine(y1, y2, rect.fLeft, rect.fRight));
    mark_dirty(lines_.size() - 1);
  }

  // Check for duplicate lines and merge them.
  if (dirty_start != static_cast<size_t>(-1)) {
    // Expand the region by one if possible.
    if (dirty_start > 0) {
      --dirty_start;
    }
    if (dirty_end + 1 < lines_.size()) {
      ++dirty_end;
    }
    for (auto i = lines_.begin() + dirty_start;
         i < lines_.begin() + dirty_end;) {
      auto& line = *i;
      auto& next = *(i + 1);
      if (line.bottom == next.top && line.spansEqual(next)) {
        --dirty_end;
        next.top = line.top;
        i = removeLine(i);
      } else {
        ++i;
      }
    }
  }
}

std::vector<SkIRect> DlRegion::getRects(bool deband) const {
  std::vector<SkIRect> rects;
  for (const auto& line : lines_) {
    for (const Span& span : *line.spans) {
      SkIRect rect{span.left, line.top, span.right, line.bottom};
      if (deband) {
        auto iter = rects.end();
        // If there is recangle previously in rect on which this one is a
        // vertical continuation, remove the previous rectangle and expand this
        // one vertically to cover the area.
        while (iter != rects.begin()) {
          --iter;
          if (iter->bottom() < rect.top()) {
            // Went too far.
            break;
          } else if (iter->bottom() == rect.top() &&
                     iter->left() == rect.left() &&
                     iter->right() == rect.right()) {
            rect.fTop = iter->fTop;
            rects.erase(iter);
            break;
          }
        }
      }
      rects.push_back(rect);
    }
  }
  return rects;
}

void DlRegion::insertLine(size_t position, SpanLine line) {
  lines_.insert(lines_.begin() + position, line);
}

DlRegion::LineVec::iterator DlRegion::removeLine(
    DlRegion::LineVec::iterator line) {
  spanvec_pool_.push_back(line->spans);
  return lines_.erase(line);
}

DlRegion::SpanLine DlRegion::makeLine(int32_t top,
                                      int32_t bottom,
                                      int32_t spanLeft,
                                      int32_t spanRight) {
  SpanVec* span_vec;
  if (!spanvec_pool_.empty()) {
    span_vec = spanvec_pool_.back();
    spanvec_pool_.pop_back();
    span_vec->clear();
  } else {
    span_vec = new SpanVec();
  }
  span_vec->push_back({spanLeft, spanRight});
  return {top, bottom, span_vec};
}

DlRegion::SpanLine DlRegion::makeLine(int32_t top,
                                      int32_t bottom,
                                      const SpanVec& spans) {
  SpanVec* span_vec;
  if (!spanvec_pool_.empty()) {
    span_vec = spanvec_pool_.back();
    spanvec_pool_.pop_back();
  } else {
    span_vec = new SpanVec();
  }
  *span_vec = spans;
  return {top, bottom, span_vec};
}

}  // namespace flutter
