#ifndef FLUTTER_TXT_CJK_UTILS_H
#define FLUTTER_TXT_CJK_UTILS_H

#include <tuple>
#include <vector>

namespace txt {

#if FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE
#include <chrono>
struct measure_time_t {
  using clock_t = std::chrono::high_resolution_clock;
  const char* label;
  clock_t::time_point start;
  explicit measure_time_t(const char* label = "");
  ~measure_time_t();
};

#define __CJK_TOKEN_CAT__(x, y) x##y
#define __CJK_TOKEN_CAT__2(x, y) __CJK_TOKEN_CAT__(x, y)
#define __CJK_MEASURE_TIME__(label) \
  txt::measure_time_t __CJK_TOKEN_CAT__2(__cjk_measure_time_, __LINE__)(label)
#define CJK_MEASURE_TIME(label) __CJK_MEASURE_TIME__(label);

#else
#define CJK_MEASURE_TIME(label)
#endif

template <typename F, typename... Args>
struct post_run_t {
  F&& post_run;
  std::tuple<Args...> fixed_args;

  explicit post_run_t(F&& fun, Args&&... args)
      : post_run(std::forward<F>(fun)),
        fixed_args(std::forward<Args>(args)...) {}

  template <typename Fun>
  auto operator()(Fun&& f) -> decltype(post_run(std::apply(f, fixed_args))) {
    auto&& result = std::apply(f, fixed_args);
    using result_type = decltype(result);
    return post_run(std::forward<result_type>(result));
  }
};

template <typename F, typename... Args>
[[maybe_unused]] post_run_t<F, Args...> make_post_run(F&& after_run,
                                                      Args&&... args) {
  return post_run_t<F, Args...>(std::forward<F>(after_run),
                                std::forward<Args>(args)...);
}

template <typename... Args>
constexpr auto make_action(Args&&... args) {
  return [packed_args = std::make_tuple(std::forward<Args>(args)...)](auto f)
      /** variadic args capturing is not available before c++20 */
      mutable { return std::apply(f, packed_args); };
}

template <typename F, typename G>
constexpr auto operator|(F&& f, G&& g) {
  return [=](auto args) mutable { return g(f(args)); };
}

// Binary search for the index of the target in arbitrary (random accessible)
// container which its elements is comparable by function [compare].
//
// Returns -1 if target not found.
template <typename F>
constexpr int bin_index_of(int size, F&& compare, bool find_closest = false) {
  if (size <= 0) {
    return -1;
  }
  int l = 0, h = size - 1;
  while (l <= h) {
    const int m = l + ((h - l) >> 1);
    const int cmp = compare(m);
    if (cmp == 0) {
      return m;
    }
    cmp < 0 ? h = m - 1 : l = m + 1;
  }
  return find_closest ? std::max(0, l - 1) : -1;
}

bool is_hard_break(uint16_t codepoint);

bool is_fullwidth(uint16_t codepoint);

bool is_cjk_ideographic_bmp(uint16_t codepoint);

uint32_t next_u16_unicode(const uint16_t* chars, size_t len, size_t& iter);

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_UTILS_H
