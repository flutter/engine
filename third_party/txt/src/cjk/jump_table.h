#ifndef FLUTTER_TXT_CJK_JUMP_TABLE_H
#define FLUTTER_TXT_CJK_JUMP_TABLE_H

#include "cjk_utils.h"

namespace txt {

template <typename T>
struct key_of_t {
  T operator()(const T& t) const { return t; }
};

template <typename T, typename K>
struct cmp_t {
  int operator()(const T& t, const K& key) const { return t - key; }
};

// Jump table for optimized binary searching, searching repetition up to
// kJumpBits (defaults to 7).
template <
    typename T,
    typename TKey = key_of_t<T>,
    typename Cmp = cmp_t<T, typename std::invoke_result<TKey, T>::type>,
    uint8_t kJumpBits = 7, /** 2^7 = 128 */
    std::enable_if_t<std::is_integral<std::invoke_result_t<TKey, T>>::value,
                     int> = 0>
class jump_table_t {
 public:
  using key_t = typename std::invoke_result<TKey, T>::type;
  using const_ref_t = const T&;
  using ref_t = T&;

  // Elements must be sorted by its key.
  explicit jump_table_t(std::vector<T>&& elements)
      : elements_(std::move(elements)) {
    compute_table();
  }

  // Elements must be sorted by its key.
  explicit jump_table_t(const std::vector<T>& elements) : elements_(elements) {
    compute_table();
  }

  jump_table_t(const jump_table_t&) = default;
  jump_table_t(jump_table_t&&) noexcept = default;
  jump_table_t& operator=(const jump_table_t&) = default;
  jump_table_t& operator=(jump_table_t&&) noexcept = default;

  // Find the index of the target given by [key], return -1 if not found.
  int find_index(key_t key) const {
    int32_t jump_offset = key >> kJumpBits;
    if (jump_offset >= (int32_t)pages_.size()) {
      jump_offset = pages_.size() - 1;
    }

    const page_t& p = pages_[jump_offset];
    int32_t elem_offset = p.elem_offset;
    int32_t elem_cnt = p.elem_cnt;

    const auto& page_first = elements_[elem_offset];
    if (cmp_(page_first, key) < 0) {
      const int32_t prev_elem_idx = elem_offset - 1;
      if (prev_elem_idx < 0) {
        return -1;
      }
      const auto& page_last = elements_[prev_elem_idx];
      if (cmp_(page_last, key) > 0) {
        return -1;
      }
      return prev_elem_idx;
    }

    // Binary search for the index of the target
    int delta_index = bin_index_of(elem_cnt, [this, elem_offset, key](int i) {
      const auto& e = elements_[i + elem_offset];
      return cmp_(e, key);
    });
    return delta_index < 0 ? -1 : delta_index + elem_offset;
  }

  // Caller must check the validity of the index.
  inline ref_t operator[](int index) { return elements_[index]; }
  inline const_ref_t operator[](int index) const { return elements_[index]; }
  inline size_t size() const { return elements_.size(); }
  inline bool empty() const { return elements_.empty(); }

 private:
  struct page_t {
    int32_t elem_offset;
    int32_t elem_cnt;
  };

  std::vector<T> elements_;
  std::vector<page_t> pages_;

  TKey key_;
  Cmp cmp_;

  void compute_table() {
    pages_.push_back({0, 0});
    size_t prev_i = 0;
    size_t i = 0;
    for (const T& e : elements_) {
      key_t jump_offset = key_(e) >> kJumpBits;
      size_t pages_size = pages_.size();
      if (jump_offset >= pages_size) {
        pages_.back().elem_cnt = i - prev_i;
        pages_.resize(jump_offset + 1, pages_.back());
        pages_.back().elem_offset = i;
        prev_i = i;
      }
      ++i;
    }
    pages_.back().elem_cnt = i - prev_i;
  }
};

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_JUMP_TABLE_H
