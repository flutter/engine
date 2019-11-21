// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_FUNCTIONAL_FUNCTION_REF_H_
#define FLUTTER_FML_FUNCTIONAL_FUNCTION_REF_H_

#include <cassert>

#include <functional>
#include <type_traits>

#include "flutter/fml/logging.h"

namespace fml {

/// A drop in replacement for std::function that doesn't own the underlying
/// memory.  This is similar to absl::FunctionRef.
///
/// This is roughly 2x-15x faster than using std::function, pass-by-value is
/// faster since the copy is so cheap.
///
/// References:
///  - https://vittorioromeo.info/index/blog/passing_functions_to_functions.html#function_view
///
/// @todo Implement constructor for function pointers, just works with
/// std::function right now.

template <typename Signature>
class FunctionRef;

template <typename R, typename... Args>
class FunctionRef<R(Args...)> final {
 public:
  /// Constructor for std::functions.
  template <typename T,
            typename = std::enable_if_t<
                // Asserting for std::functions
                std::is_function<T&(Args...)>{} &&
                // Not for function pointers
                !std::is_same<std::decay_t<T>, FunctionRef>{}>>
  FunctionRef(T&& x) : ptr_{(void*)std::addressof(x)} {
    call_ = [](void* ptr, Args... xs) -> R {
      return (*reinterpret_cast<std::add_pointer_t<T>>(ptr))(
          std::forward<Args>(xs)...);
    };
  }

  decltype(auto) operator()(Args... xs) const {
    FML_DCHECK(call_);
    return call_(ptr_, std::forward<Args>(xs)...);
  }

 private:
  void* ptr_;
  R (*call_)(void*, Args...);
};

}  // namespace fml

#endif  // FLUTTER_FML_FUNCTIONAL_FUNCTION_REF_H_
