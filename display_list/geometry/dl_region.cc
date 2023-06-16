// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_region.h"

#include "flutter/fml/logging.h"

namespace flutter {

DlRegion::SpanBuffer::SpanBuffer(DlRegion::SpanBuffer&& m)
    : capacity_(m.capacity_), size_(m.size_), spans_(m.spans_) {
  m.size_ = 0;
  m.capacity_ = 0;
  m.spans_ = nullptr;
};

DlRegion::SpanBuffer::SpanBuffer(const DlRegion::SpanBuffer& m)
    : capacity_(m.capacity_), size_(m.size_) {
  if (m.spans_ == nullptr) {
    spans_ = nullptr;
  } else {
    spans_ = static_cast<Span*>(std::malloc(capacity_ * sizeof(Span)));
    memcpy(spans_, m.spans_, size_ * sizeof(Span));
  }
};

DlRegion::SpanBuffer::~SpanBuffer() {
  free(spans_);
}

void DlRegion::SpanBuffer::reserve(size_t capacity) {
  if (capacity_ < capacity) {
    spans_ = static_cast<Span*>(std::realloc(spans_, capacity * sizeof(Span)));
    capacity_ = capacity;
  }
}

DlRegion::SpanChunkHandle DlRegion::SpanBuffer::storeChunk(const Span* begin,
                                                           const Span* end) {
  size_t chunk_size = end - begin;
  size_t min_capacity = size_ + chunk_size + 1;
  if (capacity_ < min_capacity) {
    size_t new_capacity = std::max(min_capacity, capacity_ * 2);
    new_capacity = std::max(new_capacity, size_t(512));
    reserve(new_capacity);
  }
  SpanChunkHandle res = size_;
  size_ += chunk_size + 1;
  setChunkSize(res, chunk_size);

  auto* dst = spans_ + res + 1;
  memmove(dst, begin, chunk_size * sizeof(Span));

  return res;
}

size_t DlRegion::SpanBuffer::getChunkSize(SpanChunkHandle handle) const {
  FML_DCHECK(handle < size_);
  return spans_[handle].left;
}

void DlRegion::SpanBuffer::setChunkSize(SpanChunkHandle handle, size_t size) {
  FML_DCHECK(handle < size_);
  FML_DCHECK(spans_ != nullptr);
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  spans_[handle].left = size;
}

void DlRegion::SpanBuffer::getSpans(SpanChunkHandle handle,
                                    const DlRegion::Span*& begin,
                                    const DlRegion::Span*& end) const {
  FML_DCHECK(handle < size_);
  begin = spans_ + handle + 1;
  end = begin + getChunkSize(handle);
}

DlRegion::DlRegion(const std::vector<SkIRect>& rects) {
  setRects(rects);
}

DlRegion::DlRegion() {}

DlRegion::DlRegion(const SkIRect& rect) : bounds_(rect) {
  Span span{rect.left(), rect.right()};
  lines_.push_back(makeLine(rect.top(), rect.bottom(), &span, &span + 1));
}

bool DlRegion::spansEqual(SpanLine& line,
                          const Span* begin,
                          const Span* end) const {
  const Span *our_begin, *our_end;
  span_buffer_.getSpans(line.chunk_handle, our_begin, our_end);
  size_t our_size = our_end - our_begin;
  size_t their_size = end - begin;
  if (our_size != their_size) {
    return false;
  }

  return memcmp(our_begin, begin, our_size * sizeof(Span)) == 0;
}

DlRegion::SpanLine DlRegion::makeLine(int32_t top,
                                      int32_t bottom,
                                      const SpanVec& v) {
  return makeLine(top, bottom, v.data(), v.data() + v.size());
}

DlRegion::SpanLine DlRegion::makeLine(int32_t top,
                                      int32_t bottom,
                                      const Span* begin,
                                      const Span* end) {
  auto handle = span_buffer_.storeChunk(begin, end);
  return {top, bottom, handle};
}

size_t DlRegion::unionLineSpans(std::vector<Span>& res,
                                const SpanBuffer& a_buffer,
                                SpanChunkHandle a_handle,
                                const SpanBuffer& b_buffer,
                                SpanChunkHandle b_handle) {
  const Span *begin1, *end1;
  a_buffer.getSpans(a_handle, begin1, end1);

  const Span *begin2, *end2;
  b_buffer.getSpans(b_handle, begin2, end2);

  size_t min_size = (end1 - begin1) + (end2 - begin2);
  if (res.size() < min_size) {
    res.resize(min_size);
  }

  // Pointer to the next span to be written.
  Span* new_span = res.data();

  while (true) {
    if (begin1->right < begin2->left) {
      *new_span++ = *begin1++;
      if (begin1 == end1) {
        break;
      }
    } else if (begin2->right < begin1->left) {
      *new_span++ = *begin2++;
      if (begin2 == end2) {
        break;
      }
    } else {
      break;
    }
  }

  Span current_span{0, 0};
  while (begin1 != end1 && begin2 != end2) {
    if (current_span.left == current_span.right) {
      if (begin1->right < begin2->left) {
        *new_span++ = *begin1++;
      } else if (begin2->right < begin1->left) {
        *new_span++ = *begin2++;
      } else {
        current_span = {std::min(begin1->left, begin2->left),
                        std::max(begin1->right, begin2->right)};
        begin1++;
        begin2++;
      }
    } else if (current_span.right >= begin1->left) {
      current_span.right = std::max(current_span.right, begin1->right);
      ++begin1;
    } else if (current_span.right >= begin2->left) {
      current_span.right = std::max(current_span.right, begin2->right);
      ++begin2;
    } else {
      *new_span++ = current_span;
      current_span.left = current_span.right = 0;
    }
  }

  FML_DCHECK(begin1 == end1 || begin2 == end2);

  if (current_span.left != current_span.right) {
    while (begin1 != end1 && current_span.right >= begin1->left) {
      current_span.right = std::max(current_span.right, begin1->right);
      ++begin1;
    }
    while (begin2 != end2 && current_span.right >= begin2->left) {
      current_span.right = std::max(current_span.right, begin2->right);
      ++begin2;
    }

    *new_span = current_span;
    ++new_span;
  }

  // At most one of these loops will execute
  while (begin1 != end1) {
    *new_span++ = *begin1++;
  }
  while (begin2 != end2) {
    *new_span++ = *begin2++;
  }

  FML_DCHECK(begin1 == end1 && begin2 == end2);

  return new_span - res.data();
}

size_t DlRegion::intersectLineSpans(std::vector<Span>& res,
                                    const SpanBuffer& a_buffer,
                                    SpanChunkHandle a_handle,
                                    const SpanBuffer& b_buffer,
                                    SpanChunkHandle b_handle) {
  const Span *begin1, *end1;
  a_buffer.getSpans(a_handle, begin1, end1);

  const Span *begin2, *end2;
  b_buffer.getSpans(b_handle, begin2, end2);

  // Worst case scenario, interleaved overlapping spans
  //   AAAA  BBBB  CCCC
  // XXX  YYYY  XXXX
  size_t min_size = (end1 - begin1) + (end2 - begin2) - 1;
  if (res.size() < min_size) {
    res.resize(min_size);
  }

  // Pointer to the next span to be written.
  Span* new_span = res.data();

  while (begin1 != end1 && begin2 != end2) {
    if (begin1->right <= begin2->left) {
      ++begin1;
    } else if (begin2->right <= begin1->left) {
      ++begin2;
    } else {
      int32_t left = std::max(begin1->left, begin2->left);
      int32_t right = std::min(begin1->right, begin2->right);
      FML_DCHECK(left < right);
      FML_DCHECK(new_span < res.data() + res.size());
      *new_span++ = {left, right};
      if (begin1->right == right) {
        ++begin1;
      }
      if (begin2->right == right) {
        ++begin2;
      }
    }
  }

  return new_span - res.data();
}

void DlRegion::setRects(const std::vector<SkIRect>& unsorted_rects) {
  // setRects can only be called on empty regions.
  FML_DCHECK(lines_.empty());

  size_t count = unsorted_rects.size();
  std::vector<const SkIRect*> rects(count);
  for (size_t i = 0; i < count; i++) {
    rects[i] = &unsorted_rects[i];
    bounds_.join(unsorted_rects[i]);
  }
  std::sort(rects.begin(), rects.end(), [](const SkIRect* a, const SkIRect* b) {
    if (a->top() < b->top()) {
      return true;
    }
    if (a->top() > b->top()) {
      return false;
    }
    return a->left() < b->left();
  });

  size_t active_end = 0;
  size_t next_rect = 0;
  int32_t cur_y = std::numeric_limits<int32_t>::min();
  SpanVec working_spans;

#ifdef DlRegion_DO_STATS
  size_t active_rect_count = 0;
  size_t span_count = 0;
  int pass_count = 0;
  int line_count = 0;
#endif

  while (next_rect < count || active_end > 0) {
    // First prune passed rects out of the active list
    size_t preserve_end = 0;
    for (size_t i = 0; i < active_end; i++) {
      const SkIRect* r = rects[i];
      if (r->bottom() > cur_y) {
        rects[preserve_end++] = r;
      }
    }
    active_end = preserve_end;

    // If we have no active rects any more, jump to the top of the
    // next available input rect.
    if (active_end == 0) {
      if (next_rect >= count) {
        // No active rects and no more rects to bring in. We are done.
        break;
      }
      cur_y = rects[next_rect]->top();
    }

    // Next, insert any new rects we've reached into the active list
    while (next_rect < count) {
      const SkIRect* r = rects[next_rect];
      if (r->isEmpty()) {
        continue;
      }
      if (r->top() > cur_y) {
        break;
      }
      // We now know that we will be inserting this rect into active list
      next_rect++;
      size_t insert_at = active_end++;
      while (insert_at > 0) {
        const SkIRect* ir = rects[insert_at - 1];
        if (ir->left() <= r->left()) {
          break;
        }
        rects[insert_at--] = ir;
      }
      rects[insert_at] = r;
    }

    // We either preserved some rects in the active list or added more from
    // the remaining input rects, or we would have exited the loop above.
    FML_DCHECK(active_end != 0);
    working_spans.clear();
    FML_DCHECK(working_spans.empty());

#ifdef DlRegion_DO_STATS
    active_rect_count += active_end;
    pass_count++;
#endif

    // [start_x, end_x) always represents a valid span to be inserted
    // [cur_y, end_y) is the intersecting range over which all spans are valid
    int32_t start_x = rects[0]->left();
    int32_t end_x = rects[0]->right();
    int32_t end_y = rects[0]->bottom();
    for (size_t i = 1; i < active_end; i++) {
      const SkIRect* r = rects[i];
      if (r->left() > end_x) {
        working_spans.emplace_back(start_x, end_x);
        start_x = r->left();
        end_x = r->right();
      } else if (end_x < r->right()) {
        end_x = r->right();
      }
      if (end_y > r->bottom()) {
        end_y = r->bottom();
      }
    }
    working_spans.emplace_back(start_x, end_x);

    // end_y must not pass by the top of the next input rect
    if (next_rect < count && end_y > rects[next_rect]->top()) {
      end_y = rects[next_rect]->top();
    }

    // If all of the rules above work out, we should never collapse the
    // current range of Y coordinates to empty
    FML_DCHECK(end_y > cur_y);

    if (!lines_.empty() && lines_.back().bottom == cur_y &&
        spansEqual(lines_.back(), working_spans.data(),
                   working_spans.data() + working_spans.size())) {
      lines_.back().bottom = end_y;
    } else {
#ifdef DlRegion_DO_STATS
      span_count += working_spans.size();
      line_count++;
#endif
      lines_.push_back(makeLine(cur_y, end_y, working_spans));
    }
    cur_y = end_y;
  }

#ifdef DlRegion_DO_STATS
  double span_avg = ((double)span_count) / line_count;
  double active_avg = ((double)active_rect_count) / pass_count;
  FML_LOG(ERROR) << lines_.size() << " lines for " << count
                 << " input rects, avg " << span_avg
                 << " spans per line and avg " << active_avg
                 << " active rects per loop";
#endif
}

void DlRegion::appendLine(int32_t top,
                          int32_t bottom,
                          const Span* begin,
                          const Span* end) {
  if (lines_.empty()) {
    lines_.push_back(makeLine(top, bottom, begin, end));
  } else {
    if (lines_.back().bottom == top && spansEqual(lines_.back(), begin, end)) {
      lines_.back().bottom = bottom;
    } else {
      lines_.push_back(makeLine(top, bottom, begin, end));
    }
  }
}

DlRegion DlRegion::MakeUnion(const DlRegion& a, const DlRegion& b) {
  if (a.isSimple() && a.bounds_.contains(b.bounds_)) {
    return a;
  } else if (b.isSimple() && b.bounds_.contains(a.bounds_)) {
    return b;
  }

  DlRegion res;
  res.bounds_ = a.bounds_;
  res.bounds_.join(b.bounds_);
  res.span_buffer_.reserve(a.span_buffer_.capacity() +
                           b.span_buffer_.capacity());

  auto& lines = res.lines_;
  lines.reserve(a.lines_.size() + b.lines_.size());

  auto a_it = a.lines_.begin();
  auto b_it = b.lines_.begin();
  auto a_end = a.lines_.end();
  auto b_end = b.lines_.end();

  auto& a_buffer = a.span_buffer_;
  auto& b_buffer = b.span_buffer_;

  std::vector<Span> tmp;

  int32_t cur_top = std::numeric_limits<int32_t>::min();

  while (a_it != a_end && b_it != b_end) {
    auto a_top = std::max(cur_top, a_it->top);
    auto b_top = std::max(cur_top, b_it->top);
    if (a_it->bottom <= b_top) {
      res.appendLine(a_top, a_it->bottom, a_buffer, a_it->chunk_handle);
      ++a_it;
    } else if (b_it->bottom <= a_top) {
      res.appendLine(b_top, b_it->bottom, b_buffer, b_it->chunk_handle);
      ++b_it;
    } else {
      if (a_top < b_top) {
        res.appendLine(a_top, b_top, a_buffer, a_it->chunk_handle);
        cur_top = b_top;
        if (cur_top == a_it->bottom) {
          ++a_it;
        }
      } else if (b_top < a_top) {
        res.appendLine(b_top, a_top, b_buffer, b_it->chunk_handle);
        cur_top = a_top;
        if (cur_top == b_it->bottom) {
          ++b_it;
        }
      } else {
        auto new_bottom = std::min(a_it->bottom, b_it->bottom);
        FML_DCHECK(a_top == b_top);
        FML_DCHECK(new_bottom > a_top);
        FML_DCHECK(new_bottom > b_top);
        auto size = unionLineSpans(tmp, a_buffer, a_it->chunk_handle, b_buffer,
                                   b_it->chunk_handle);
        res.appendLine(a_top, new_bottom, tmp.data(), tmp.data() + size);
        cur_top = new_bottom;
        if (cur_top == a_it->bottom) {
          ++a_it;
        }
        if (cur_top == b_it->bottom) {
          ++b_it;
        }
      }
    }
  }

  FML_DCHECK(a_it == a_end || b_it == b_end);

  while (a_it != a_end) {
    auto a_top = std::max(cur_top, a_it->top);
    res.appendLine(a_top, a_it->bottom, a_buffer, a_it->chunk_handle);
    ++a_it;
  }

  while (b_it != b_end) {
    auto b_top = std::max(cur_top, b_it->top);
    res.appendLine(b_top, b_it->bottom, b_buffer, b_it->chunk_handle);
    ++b_it;
  }

  return res;
}

DlRegion DlRegion::MakeIntersection(const DlRegion& a, const DlRegion& b) {
  if (!SkIRect::Intersects(a.bounds_, b.bounds_)) {
    return DlRegion();
  } else if (a.isSimple() && b.isSimple()) {
    SkIRect r(a.bounds_);
    auto res = r.intersect(b.bounds_);
    FML_DCHECK(res);
    return DlRegion(r);
  } else if (a.isSimple() && a.bounds_.contains(b.bounds_)) {
    return b;
  } else if (b.isSimple() && b.bounds_.contains(a.bounds_)) {
    return a;
  }

  DlRegion res;
  res.span_buffer_.reserve(
      std::max(a.span_buffer_.capacity(), b.span_buffer_.capacity()));

  auto& lines = res.lines_;
  lines.reserve(std::min(a.lines_.size(), b.lines_.size()));

  auto a_it = a.lines_.begin();
  auto a_end = a.lines_.end();
  auto b_it = b.lines_.begin();
  auto b_end = b.lines_.end();

  // When intersecting single rectangle with a complex region use binary
  // search to find the first line that intersects the rectangle.
  if (b_it == b_end - 1) {
    a_it = std::upper_bound(
        a_it, a_end, b_it->top,
        [](int32_t top, const SpanLine& line) { return top < line.bottom; });
  } else if (a_it == a_end - 1) {
    b_it = std::upper_bound(
        b_it, b_end, a_it->top,
        [](int32_t top, const SpanLine& line) { return top < line.bottom; });
  }

  auto& a_buffer = a.span_buffer_;
  auto& b_buffer = b.span_buffer_;

  std::vector<Span> tmp;

  int32_t cur_top = std::numeric_limits<int32_t>::min();

  while (a_it != a_end && b_it != b_end) {
    auto a_top = std::max(cur_top, a_it->top);
    auto b_top = std::max(cur_top, b_it->top);
    if (a_it->bottom <= b_top) {
      ++a_it;
    } else if (b_it->bottom <= a_top) {
      ++b_it;
    } else {
      auto top = std::max(a_top, b_top);
      auto bottom = std::min(a_it->bottom, b_it->bottom);
      FML_DCHECK(top < bottom);
      auto size = intersectLineSpans(tmp, a_buffer, a_it->chunk_handle,
                                     b_buffer, b_it->chunk_handle);
      if (size > 0) {
        res.appendLine(top, bottom, tmp.data(), tmp.data() + size);
        res.bounds_.join(SkIRect::MakeLTRB(
            tmp.data()->left, top, (tmp.data() + size - 1)->right, bottom));
      }
      cur_top = bottom;
      if (cur_top == a_it->bottom) {
        ++a_it;
      }
      if (cur_top == b_it->bottom) {
        ++b_it;
      }
    }
  }
  FML_DCHECK(a_it == a_end || b_it == b_end);
  return res;
}

std::vector<SkIRect> DlRegion::getRects(bool deband) const {
  std::vector<SkIRect> rects;
  if (isEmpty()) {
    return rects;
  } else if (isSimple()) {
    rects.push_back(bounds_);
    return rects;
  }

  size_t rect_count = 0;
  size_t previous_span_end = 0;
  for (const auto& line : lines_) {
    rect_count += span_buffer_.getChunkSize(line.chunk_handle);
  }
  rects.reserve(rect_count);

  for (const auto& line : lines_) {
    const Span *span_begin, *span_end;
    span_buffer_.getSpans(line.chunk_handle, span_begin, span_end);
    for (const auto* span = span_begin; span < span_end; ++span) {
      SkIRect rect{span->left, line.top, span->right, line.bottom};
      if (deband) {
        auto iter = rects.begin() + previous_span_end;
        // If there is rectangle previously in rects on which this one is a
        // vertical continuation, remove the previous rectangle and expand
        // this one vertically to cover the area.
        while (iter != rects.begin()) {
          --iter;
          if (iter->bottom() < rect.top()) {
            // Went all the way to previous span line.
            break;
          } else if (iter->left() == rect.left() &&
                     iter->right() == rect.right()) {
            FML_DCHECK(iter->bottom() == rect.top());
            rect.fTop = iter->fTop;
            rects.erase(iter);
            --previous_span_end;
            break;
          }
        }
      }
      rects.push_back(rect);
    }
    previous_span_end = rects.size();
  }
  return rects;
}

DlRegion::~DlRegion() {}

bool DlRegion::isComplex() const {
  return lines_.size() > 1 ||
         (lines_.size() == 1 &&
          span_buffer_.getChunkSize(lines_.front().chunk_handle) > 1);
}

bool DlRegion::intersects(const SkIRect& rect) const {
  if (isEmpty()) {
    return false;
  }

  auto bounds_intersect = SkIRect::Intersects(bounds_, rect);

  if (isSimple()) {
    return bounds_intersect;
  }

  if (!bounds_intersect) {
    return false;
  }

  auto it = std::upper_bound(
      lines_.begin(), lines_.end(), rect.fTop,
      [](int32_t i, const SpanLine& line) { return i < line.bottom; });

  while (it != lines_.end() && it->top < rect.fBottom) {
    FML_DCHECK(rect.fTop < it->bottom || it->top < rect.fBottom);
    const Span *begin, *end;
    span_buffer_.getSpans(it->chunk_handle, begin, end);
    while (begin != end && begin->left < rect.fRight) {
      if (begin->right > rect.fLeft) {
        return true;
      }
      ++begin;
    }
    ++it;
  }

  return false;
}

bool DlRegion::spansIntersect(const Span* begin1,
                              const Span* end1,
                              const Span* begin2,
                              const Span* end2) {
  while (begin1 != end1 && begin2 != end2) {
    if (begin1->right <= begin2->left) {
      ++begin1;
    } else if (begin2->right <= begin1->left) {
      ++begin2;
    } else {
      return true;
    }
  }
  return false;
}

bool DlRegion::intersects(const DlRegion& region) const {
  if (isEmpty() || region.isEmpty()) {
    return false;
  }

  auto our_complex = isComplex();
  auto their_complex = region.isComplex();
  auto bounds_intersect = SkIRect::Intersects(bounds_, region.bounds_);

  if (!our_complex && !their_complex) {
    return bounds_intersect;
  }

  if (!bounds_intersect) {
    return false;
  }

  if (!our_complex) {
    return region.intersects(bounds_);
  }

  if (!their_complex) {
    return intersects(region.bounds_);
  }

  auto ours = lines_.begin();
  auto theirs = region.lines_.begin();

  while (ours != lines_.end() && theirs != region.lines_.end()) {
    if (ours->bottom <= theirs->top) {
      ++ours;
    } else if (theirs->bottom <= ours->top) {
      ++theirs;
    } else {
      FML_DCHECK(ours->top < theirs->bottom || theirs->top < ours->bottom);
      const Span *ours_begin, *ours_end;
      span_buffer_.getSpans(ours->chunk_handle, ours_begin, ours_end);
      const Span *theirs_begin, *theirs_end;
      region.span_buffer_.getSpans(theirs->chunk_handle, theirs_begin,
                                   theirs_end);
      if (spansIntersect(ours_begin, ours_end, theirs_begin, theirs_end)) {
        return true;
      }
      if (ours->bottom < theirs->bottom) {
        ++ours;
      } else {
        ++theirs;
      }
    }
  }
  return false;
}

}  // namespace flutter
