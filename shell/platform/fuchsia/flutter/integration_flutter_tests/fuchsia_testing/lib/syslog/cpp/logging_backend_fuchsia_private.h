// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

class ByteOffset {
 public:
  ByteOffset(const ByteOffset& other) : value_(other.value_), capacity_(other.capacity_) {}
  static ByteOffset FromBuffer(size_t offset, size_t capacity) {
    return ByteOffset(offset, capacity).AssertValid();
  }
  static ByteOffset Unbounded(size_t offset) { return ByteOffset(offset, -1); }
  size_t unsafe_get() const { return value_; }
  size_t capacity() const { return capacity_; }
  ByteOffset AssertAlignedTo(size_t size) const {
    assert((value_ % size) == 0);
    if (!((value_ % size) == 0)) {
      abort();
    }
    return *this;
  }
  ByteOffset operator+(size_t offset) const {
    return ByteOffset(value_ + offset, capacity_).AssertValid();
  }
  ByteOffset() = delete;

 private:
  ByteOffset(size_t value, size_t capacity) : value_(value), capacity_(capacity) {}
  ByteOffset& AssertValid() {
    if (!((capacity_ == 0) && (value_ == 0))) {
      assert(value_ < capacity_);
      if (!(value_ < capacity_)) {
        abort();
      }
    }
    return *this;
  }
  size_t value_;
  size_t capacity_;
};

template <typename T>
class WordOffset {
 public:
  WordOffset() = delete;
  WordOffset(WordOffset& other) {
    capacity_ = other.capacity_;
    value_ = other.value_;
  }
  WordOffset operator+(size_t offset) const {
    return WordOffset(value_ + offset, capacity_).AssertValid();
  }
  WordOffset operator+(WordOffset offset) const {
    return WordOffset(value_ + offset.value_, capacity_).AssertValid();
  }
  WordOffset AddPadded(const ByteOffset& byte_offset) {
    size_t needs_padding = (byte_offset.unsafe_get() % sizeof(T)) > 0;
    // Multiply by needs_padding to set padding to 0 if no padding
    // is necessary. This avoids unnecessary branching.
    return WordOffset(value_ + (byte_offset.unsafe_get() / sizeof(T)) + needs_padding, capacity_);
  }
  WordOffset begin() { return WordOffset(0, capacity_); }
  size_t capacity() { return capacity_; }
  static WordOffset FromByteOffset(const ByteOffset& value) {
    return WordOffset(value.AssertAlignedTo(sizeof(T)).unsafe_get() / sizeof(T),
                      value.capacity() / sizeof(T));
  }
  size_t unsafe_get() const { return value_; }
  bool in_bounds(WordOffset<T> offset) { return !((offset.value_ + value_) >= capacity_); }
  ByteOffset ToByteOffset() const {
    return ByteOffset::FromBuffer(value_ * sizeof(T), capacity_ * sizeof(T));
  }
  void reset() { value_ = 0; }
  static WordOffset invalid() { return WordOffset(0, 0); }

 private:
  WordOffset(size_t value, size_t capacity) : capacity_(capacity), value_(value) {}
  WordOffset& AssertValid() {
    if (!((capacity_ == 0) && (value_ == 0))) {
      if (!(value_ < capacity_)) {
        abort();
      }
      assert(value_ < capacity_);
    }
    return *this;
  }
  size_t capacity_;
  size_t value_;
};

template <typename T>
WordOffset<T> WritePaddedInternal(T* buffer, const void* msg, const ByteOffset& length) {
  size_t needs_padding = (length.unsafe_get() % sizeof(T)) > 0;
  size_t padding = sizeof(T) - (length.unsafe_get() % sizeof(T));
  // Multiply by needs_padding to set padding to 0 if no padding
  // is necessary. This avoids unnecessary branching.
  padding *= needs_padding;
  // If we added padding -- zero the padding bytes in a single write operation
  size_t is_nonzero_length = length.unsafe_get() != 0;
  size_t eof_in_bytes = length.unsafe_get() + padding;
  size_t eof_in_words = eof_in_bytes / sizeof(T);
  size_t last_word = eof_in_words - is_nonzero_length;
  // Set the last word in the buffer to zero before writing
  // the data to it if we added padding. If we didn't add padding,
  // multiply by 1 which ends up writing back the current contents of that word
  // resulting in a NOP.
  buffer[last_word] *= !needs_padding;
  memcpy(buffer, msg, length.unsafe_get());
  return WordOffset<T>::FromByteOffset(ByteOffset::Unbounded(length.unsafe_get() + padding));
}
