// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_region.h"

#include "flutter/fml/logging.h"

namespace flutter {

class SpanBuffer {
 public:
  static const uint32_t kDefaultCapacity;

  SpanBuffer() {
    free_handles_.reserve(256);
    chunks_.reserve(256);
    data_.reserve(2048);
  }

  ~SpanBuffer() {
    // fprintf(stderr, "Allocated size %zu, Handles: %zu, Free handles: %zu\n",
    // data_.size(), chunks_.size(), free_handles_.size());
  }

  SpanChunkHandle allocateChunk(uint32_t capacity = kDefaultCapacity) {
    for (auto it = free_handles_.rbegin(); it != free_handles_.rend(); ++it) {
      SpanChunkInfo& info = chunks_[*it];
      if (info.capacity >= capacity) {
        SpanChunkHandle handle = *it;
        if (free_handles_.size() > 1) {
          *it = free_handles_.back();
          free_handles_.pop_back();
        } else {
          free_handles_.clear();
        }
        ++info.ref_count;
        info.size = 0;
        return handle;
      }
    }
    return allocateWithCapacity(capacity);
  }

  SpanChunkHandle duplicateChunk(SpanChunkHandle handle) {
    SpanChunkInfo& info = chunks_[handle];
    ++info.ref_count;
    return handle;
  }

  SpanChunkHandle copyChunk(DlRegion::Span* begin, DlRegion::Span* end) {
    size_t size = end - begin;
    SpanChunkHandle new_handle = allocateChunk(size * 2);
    SpanChunkInfo& info = chunks_[new_handle];
    std::memcpy(&data_[info.offset], begin, size * sizeof(DlRegion::Span));
    info.size = size;
    return new_handle;
  }

  void freeChunk(SpanChunkHandle handle) {
    SpanChunkInfo& info = chunks_[handle];
    FML_DCHECK(info.ref_count > 0);
    --info.ref_count;
    if (info.ref_count == 0) {
      free_handles_.push_back(handle);
    }
  }

  size_t getChunkSize(SpanChunkHandle handle) { return chunks_[handle].size; }

  void updateChunkSize(SpanChunkHandle handle, size_t size) {
    FML_DCHECK(chunks_[handle].ref_count == 1);
    SpanChunkInfo& info = chunks_[handle];
    FML_DCHECK(size <= info.capacity);
    info.size = size;
  }

  void getSpans(SpanChunkHandle handle,
                DlRegion::Span*& begin,
                DlRegion::Span*& end) {
    const SpanChunkInfo& info = chunks_[handle];
    begin = &data_[info.offset];
    end = begin + info.size;
  }

  void eraseSpans(SpanChunkHandle handle, size_t begin, size_t end) {
    FML_DCHECK(chunks_[handle].ref_count == 1);
    SpanChunkInfo& info = chunks_[handle];

    if (end == info.size) {
      info.size = begin;
      return;
    }

    DlRegion::Span* span_begin = &data_[info.offset];
    DlRegion::Span* span_end = span_begin + info.size;
    DlRegion::Span* erase_begin = span_begin + begin;
    DlRegion::Span* erase_end = span_begin + end;
    std::memmove(erase_begin, erase_end,
                 (span_end - erase_end) * sizeof(DlRegion::Span));
    info.size -= end - begin;
  }

  void appendSpan(SpanChunkHandle handle, const DlRegion::Span& span) {
    FML_DCHECK(chunks_[handle].ref_count == 1);
    ensureSpace(handle);
    SpanChunkInfo& info = chunks_[handle];
    DlRegion::Span* begin = &data_[info.offset];
    DlRegion::Span* end = begin + info.size;
    *end = span;
    ++info.size;
  }

  void insertSpan(SpanChunkHandle handle,
                  size_t index,
                  const DlRegion::Span& span) {
    FML_DCHECK(chunks_[handle].ref_count == 1);
    ensureSpace(handle);
    SpanChunkInfo& info = chunks_[handle];
    DlRegion::Span* begin = &data_[info.offset];
    DlRegion::Span* end = begin + info.size;
    DlRegion::Span* insert = begin + index;
    std::memmove(insert + 1, insert, (end - insert) * sizeof(DlRegion::Span));
    *insert = span;
    ++info.size;
  }

  SpanChunkHandle ensureChunkWritable(SpanChunkHandle handle) {
    SpanChunkInfo info = chunks_[handle];
    FML_DCHECK(info.ref_count > 0);
    if (info.ref_count == 1) {
      return handle;
    } else {
      SpanChunkHandle new_handle = allocateChunk(info.capacity);
      SpanChunkInfo& new_info = chunks_[new_handle];
      new_info.size = info.size;
      std::memmove(&data_[new_info.offset], &data_[info.offset],
                   info.size * sizeof(DlRegion::Span));
      --chunks_[handle].ref_count;
      return new_handle;
    }
  }

 private:
  static size_t round_to_8(size_t x) { return ((x + 7) & (-8)); }

  SpanChunkHandle allocateWithCapacity(uint32_t capacity) {
    capacity = round_to_8(std::max(capacity, kDefaultCapacity));
    chunks_.push_back({static_cast<uint32_t>(data_.size()), 0, capacity, 1});
    data_.resize(data_.size() + capacity);
    return chunks_.size() - 1;
  }

  void ensureSpace(SpanChunkHandle handle) {
    SpanChunkInfo& info = chunks_[handle];
    if (info.size == info.capacity) {
      auto previous_info = info;
      info.capacity *= 2;
      auto prev_offset = info.offset;
      info.offset = data_.size();
      data_.resize(data_.size() + info.capacity);
      std::memmove(&data_[info.offset], &data_[prev_offset],
                   info.size * sizeof(DlRegion::Span));
      chunks_.push_back({
          previous_info.offset,
          0,
          previous_info.capacity,
          0,
      });
      free_handles_.push_back(chunks_.size() - 1);
    }
  }

  struct SpanChunkInfo {
    /// Offset from the beginning of the buffer.
    uint32_t offset;
    uint32_t size;
    uint32_t capacity;
    int32_t ref_count;
  };

  std::vector<DlRegion::Span> data_;
  std::vector<SpanChunkInfo> chunks_;
  std::vector<SpanChunkHandle> free_handles_;
};

const uint32_t SpanBuffer::kDefaultCapacity = 16;

DlRegion::DlRegion() : span_buffer_(std::make_shared<SpanBuffer>()) {}

DlRegion::DlRegion(std::vector<SkIRect>&& rects)
    : span_buffer_(std::make_shared<SpanBuffer>()) {
  // If SpanLines can not be memmoved `addRect` would be significantly slower
  // due to cost of inserting and removing elements from the `lines_` vector.
  static_assert(std::is_trivially_constructible<SpanLine>::value,
                "SpanLine must be trivially constructible.");
  addRects(std::move(rects));
}

DlRegion::~DlRegion() {
  for (auto& line : lines_) {
    span_buffer_->freeChunk(line.chunk_handle);
  }
}

DlRegion::DlRegion(const DlRegion& region, bool share_buffer)
    : span_buffer_(share_buffer
                       ? region.span_buffer_
                       : std::make_shared<SpanBuffer>(*region.span_buffer_)) {
  lines_ = region.lines_;
  for (auto& line : lines_) {
    line.chunk_handle = span_buffer_->duplicateChunk(line.chunk_handle);
  }
  bounds_ = region.bounds_;
}

bool DlRegion::spansIntersect(const Span* begin1,
                              const Span* end1,
                              const Span* begin2,
                              const Span* end2) const {
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

bool DlRegion::intersects(const SkIRect& rect) const {
  if (isEmpty()) {
    return false;
  }

  auto bounds_intersect = SkIRect::Intersects(bounds_, rect);

  if (!isComplex()) {
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
    Span *begin, *end;
    span_buffer_->getSpans(it->chunk_handle, begin, end);
    while (begin != end && begin->left < rect.fRight) {
      if (begin->right > rect.fLeft && begin->left < rect.fRight) {
        return true;
      }
      ++begin;
    }
    ++it;
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
      Span *ours_begin, *ours_end;
      span_buffer_->getSpans(ours->chunk_handle, ours_begin, ours_end);
      Span *theirs_begin, *theirs_end;
      region.span_buffer_->getSpans(theirs->chunk_handle, theirs_begin,
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

std::vector<SkIRect> DlRegion::getRects(bool deband) const {
  std::vector<SkIRect> rects;
  size_t rect_count = 0;
  size_t previous_span_end = 0;
  for (const auto& line : lines_) {
    rect_count += span_buffer_->getChunkSize(line.chunk_handle);
  }
  rects.reserve(rect_count);

  for (const auto& line : lines_) {
    Span *span_begin, *span_end;
    span_buffer_->getSpans(line.chunk_handle, span_begin, span_end);
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

void DlRegion::SpanLine::insertSpan(SpanBuffer& span_buffer,
                                    int32_t left,
                                    int32_t right) {
  Span *span_begin, *span_end;
  chunk_handle = span_buffer.ensureChunkWritable(chunk_handle);
  span_buffer.getSpans(chunk_handle, span_begin, span_end);
  size_t size = span_end - span_begin;
  for (size_t i = 0; i < size; ++i) {
    Span& span = span_begin[i];
    if (right < span.left) {
      span_buffer.insertSpan(chunk_handle, i, {left, right});
      return;
    }
    if (left > span.right) {
      continue;
    }
    size_t last_index = i;
    while (last_index + 1 < size && right >= span_begin[last_index + 1].left) {
      ++last_index;
    }
    span.left = std::min(span.left, left);
    span.right = std::max(span_begin[last_index].right, right);
    if (last_index > i) {
      span_buffer.eraseSpans(chunk_handle, i + 1, last_index + 1);
    }
    return;
  }

  span_buffer.appendSpan(chunk_handle, {left, right});
}

void DlRegion::SpanLine::insertSpans(SpanBuffer& buffer,
                                     const Span* begin,
                                     const Span* end) {
  for (auto* span = begin; span != end; ++span) {
    insertSpan(buffer, span->left, span->right);
  }
}

bool DlRegion::SpanLine::spansEqual(SpanBuffer& buffer,
                                    const SpanLine& l2) const {
  FML_DCHECK(this != &l2);

  if (chunk_handle == l2.chunk_handle) {
    return true;
  }

  Span *our_begin, *our_end, *outher_begin, *other_end;
  buffer.getSpans(chunk_handle, our_begin, our_end);
  buffer.getSpans(l2.chunk_handle, outher_begin, other_end);

  size_t our_size = our_end - our_begin;
  size_t other_size = other_end - outher_begin;

  if (our_size != other_size) {
    return false;
  }
  return memcmp(our_begin, outher_begin, our_size * sizeof(Span)) == 0;
}

void DlRegion::insertLine(size_t position, SpanLine line) {
  lines_.insert(lines_.begin() + position, line);
}

DlRegion::LineVec::iterator DlRegion::removeLine(
    DlRegion::LineVec::iterator line) {
  span_buffer_->freeChunk(line->chunk_handle);
  return lines_.erase(line);
}

DlRegion::SpanLine DlRegion::makeLine(int32_t top,
                                      int32_t bottom,
                                      int32_t spanLeft,
                                      int32_t spanRight) {
  auto handle = span_buffer_->allocateChunk();
  span_buffer_->appendSpan(handle, {spanLeft, spanRight});
  return {top, bottom, handle};
}

DlRegion::SpanLine DlRegion::mergeLines(int32_t top,
                                        int32_t bottom,
                                        SpanChunkHandle our_handle,
                                        SpanBuffer* their_span_buffer,
                                        SpanChunkHandle their_handle) {
  Span *begin2, *end2;
  their_span_buffer->getSpans(their_handle, begin2, end2);

  auto handle = span_buffer_->allocateChunk(
      span_buffer_->getChunkSize(our_handle) + (end2 - begin2));
  Span *begin1, *end1;
  span_buffer_->getSpans(our_handle, begin1, end1);

  Span *begin, *end;
  span_buffer_->getSpans(handle, begin, end);

  while (true) {
    if (begin1->right < begin2->left - 1) {
      *end = *begin1;
      ++begin1;
      ++end;
      if (begin1 == end1) {
        break;
      }
    } else if (begin2->right < begin1->left) {
      *end = *begin2;
      ++begin2;
      ++end;
      if (begin2 == end2) {
        break;
      }
    } else {
      break;
    }
  }

  Span currentSpan{0, 0};
  while (begin1 != end1 && begin2 != end2) {
    if (currentSpan.left == currentSpan.right) {
      if (begin1->right < begin2->left - 1) {
        *end = *begin1;
        ++begin1;
        ++end;
      } else if (begin2->right < begin1->left) {
        *end = *begin2;
        ++begin2;
        ++end;
      } else if (begin1->left == begin2->left) {
        currentSpan.left = begin1->left;
        currentSpan.right = std::max(begin1->right, begin2->right);
        ++begin1;
        ++begin2;
      } else if (begin1->left < begin2->left) {
        currentSpan.left = begin1->left;
        currentSpan.right = begin1->right;
        ++begin1;
      } else {
        currentSpan.left = begin2->left;
        currentSpan.right = begin2->right;
        ++begin2;
      }
    } else if (currentSpan.right >= begin1->left) {
      currentSpan.right = std::max(currentSpan.right, begin1->right);
      ++begin1;
    } else if (currentSpan.right >= begin2->left) {
      currentSpan.right = std::max(currentSpan.right, begin2->right);
      ++begin2;
    } else {
      *end = currentSpan;
      ++end;
      currentSpan.left = currentSpan.right = 0;
    }
  }

  if (currentSpan.left != currentSpan.right) {
    while (begin1 != end1 && currentSpan.right >= begin1->left) {
      currentSpan.right = std::max(currentSpan.right, begin1->right);
      ++begin1;
    }
    while (begin2 != end2 && currentSpan.right >= begin2->left) {
      currentSpan.right = std::max(currentSpan.right, begin2->right);
      ++begin2;
    }

    *end = currentSpan;
    ++end;
  }

  FML_DCHECK(begin1 == end1 || begin2 == end2);

  while (begin1 != end1) {
    *end = *begin1;
    ++begin1;
    ++end;
  }

  while (begin2 != end2) {
    *end = *begin2;
    ++begin2;
    ++end;
  }

  span_buffer_->updateChunkSize(handle, end - begin);

  return {top, bottom, handle};
}

DlRegion::SpanLine DlRegion::duplicateLine(int32_t top,
                                           int32_t bottom,
                                           SpanBuffer* span_buffer,
                                           SpanChunkHandle handle) {
  if (span_buffer == span_buffer_.get()) {
    return {top, bottom, span_buffer_->duplicateChunk(handle)};
  } else {
    SpanChunkHandle new_handle;
    Span *span_begin, *span_end;
    span_buffer->getSpans(handle, span_begin, span_end);
    new_handle = span_buffer_->copyChunk(span_begin, span_end);
    return {top, bottom, new_handle};
  }
}

DlRegion::SpanLine DlRegion::duplicateLine(int32_t top,
                                           int32_t bottom,
                                           SpanChunkHandle handle) {
  return {top, bottom, span_buffer_->duplicateChunk(handle)};
}

void DlRegion::addRects(std::vector<SkIRect>&& rects) {
  std::sort(rects.begin(), rects.end(), [](const SkIRect& a, const SkIRect& b) {
    // Sort the rectangles by Y axis. Because the rectangles have varying
    // height, they are added to span lines in non-deterministic order and thus
    // it makes no difference if they are also sorted by the X axis.
    return a.top() < b.top();
  });

  size_t start_index = 0;

  size_t dirty_start = std::numeric_limits<size_t>::max();
  size_t dirty_end = 0;

  // Marks line as dirty. Dirty lines will be checked for equality
  // later and merged as needed.
  auto mark_dirty = [&](size_t line) {
    dirty_start = std::min(dirty_start, line);
    dirty_end = std::max(dirty_end, line);
  };

  for (const SkIRect& rect : rects) {
    if (rect.isEmpty()) {
      continue;
    }

    bounds_.join(rect);

    int32_t y1 = rect.fTop;
    int32_t y2 = rect.fBottom;

    for (size_t i = start_index; i < lines_.size() && y1 < y2; ++i) {
      SpanLine& line = lines_[i];

      if (rect.fTop >= line.bottom) {
        start_index = i;
        continue;
      }

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
        insertLine(i + 1, duplicateLine(y1, prevLineEnd, line.chunk_handle));
        continue;
      }
      FML_DCHECK(y1 == line.top);
      if (y2 < line.bottom) {
        // duplicate line
        auto new_line = duplicateLine(y2, line.bottom, line.chunk_handle);
        line.bottom = y2;
        line.insertSpan(*span_buffer_, rect.fLeft, rect.fRight);
        insertLine(i + 1, new_line);
        y1 = y2;
        mark_dirty(i);
        break;
      }
      FML_DCHECK(y2 >= line.bottom);
      line.insertSpan(*span_buffer_, rect.fLeft, rect.fRight);
      mark_dirty(i);
      y1 = line.bottom;
    }

    if (y1 < y2) {
      lines_.push_back(makeLine(y1, y2, rect.fLeft, rect.fRight));
      mark_dirty(lines_.size() - 1);
    }

    // Check for duplicate lines and merge them.
    if (dirty_start <= dirty_end) {
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
        if (line.bottom == next.top && line.spansEqual(*span_buffer_, next)) {
          --dirty_end;
          next.top = line.top;
          i = removeLine(i);
        } else {
          ++i;
        }
      }
    }
    dirty_start = std::numeric_limits<size_t>::max();
    dirty_end = 0;
  }
}

bool DlRegion::isComplex() const {
  return lines_.size() > 1 ||
         (lines_.size() == 1 &&
          span_buffer_->getChunkSize(lines_.front().chunk_handle) > 1);
}

void DlRegion::addRegion(const DlRegion& region) {
  bounds_.join(region.bounds_);

  LineVec res;
  res.reserve(lines_.size() + region.lines_.size());

  auto append_line = [&](SpanLine line) {
    if (res.empty()) {
      res.push_back(line);
    } else {
      if (res.back().bottom == line.top &&
          res.back().spansEqual(*span_buffer_, line)) {
        res.back().bottom = line.bottom;
        span_buffer_->freeChunk(line.chunk_handle);
      } else {
        res.push_back(line);
      }
    }
  };

  LineVec::iterator ours = lines_.begin();
  auto theirs_copy = region.lines_;
  LineVec::iterator theirs = theirs_copy.begin();

  auto their_span_buffer = region.span_buffer_.get();

  while (ours != lines_.end() && theirs != theirs_copy.end()) {
    if (ours->bottom <= theirs->top) {
      append_line(*ours);
      ++ours;
    } else if (theirs->bottom <= ours->top) {
      append_line(duplicateLine(theirs->top, theirs->bottom, their_span_buffer,
                                theirs->chunk_handle));
      ++theirs;
    } else {
      if (ours->top < theirs->top) {
        append_line(duplicateLine(ours->top, theirs->top, ours->chunk_handle));
        ours->top = theirs->top;
        if (ours->top == ours->bottom) {
          span_buffer_->freeChunk(ours->chunk_handle);
          ++ours;
        }
      } else if (theirs->top < ours->top) {
        append_line(duplicateLine(theirs->top, ours->top, their_span_buffer,
                                  theirs->chunk_handle));
        theirs->top = ours->top;
        if (theirs->top == theirs->bottom) {
          ++theirs;
        }
      } else {
        auto new_bottom = std::min(ours->bottom, theirs->bottom);
        FML_DCHECK(ours->top == theirs->top);
        FML_DCHECK(new_bottom > ours->top);
        FML_DCHECK(new_bottom > theirs->top);
        append_line(mergeLines(ours->top, new_bottom, ours->chunk_handle,
                               their_span_buffer, theirs->chunk_handle));
        ours->top = new_bottom;
        if (ours->top == ours->bottom) {
          span_buffer_->freeChunk(ours->chunk_handle);
          ++ours;
        }
        theirs->top = new_bottom;
        if (theirs->top == theirs->bottom) {
          ++theirs;
        }
      }
    }
  }

  FML_DCHECK(ours == lines_.end() || theirs == theirs_copy.end());

  while (ours != lines_.end()) {
    append_line(*ours);
    ++ours;
  }

  while (theirs != theirs_copy.end()) {
    append_line(duplicateLine(theirs->top, theirs->bottom, their_span_buffer,
                              theirs->chunk_handle));
    ++theirs;
  }

  lines_ = std::move(res);
}

}  // namespace flutter
