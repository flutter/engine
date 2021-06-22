// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FBL_MACROS_H_
#define FBL_MACROS_H_

#include <type_traits>

// Macro used to simplify the task of deleting all of the default copy
// constructors and assignment operators.
#define DISALLOW_COPY_ASSIGN_AND_MOVE(_class_name)     \
  _class_name(const _class_name&) = delete;            \
  _class_name(_class_name&&) = delete;                 \
  _class_name& operator=(const _class_name&) = delete; \
  _class_name& operator=(_class_name&&) = delete

// Macro used to simplify the task of deleting the non rvalue reference copy
// constructors and assignment operators.  (IOW - forcing move semantics)
#define DISALLOW_COPY_AND_ASSIGN_ALLOW_MOVE(_class_name) \
  _class_name(const _class_name&) = delete;              \
  _class_name& operator=(const _class_name&) = delete

// Macro for defining a trait that checks if a type T has a method with the
// given name. This is not as strong as using is_same to check function
// signatures, but checking this trait first gives a better static_assert
// message than the compiler warnings from is_same if the function doesn't
// exist.
// Note that the resulting trait_name will be in the namespace where the macro
// is used.
//
// Example:
//
// DECLARE_HAS_MEMBER_FN(has_bar, Bar);
// template <typename T>
// class Foo {
//   static_assert(has_bar_v<T>, "Foo classes must implement Bar()!");
//   // TODO: use 'if constexpr' to avoid this next static_assert once c++17
//   lands.
//   static_assert(is_same_v<decltype(&T::Bar), void (T::*)(int)>,
//                 "Bar must be a non-static member function with signature "
//                 "'void Bar(int)', and must be visible to Foo (either "
//                 "because it is public, or due to friendship).");
//  };
#define DECLARE_HAS_MEMBER_FN(trait_name, fn_name)                   \
  template <typename T>                                              \
  struct trait_name {                                                \
   private:                                                          \
    template <typename C>                                            \
    static std::true_type test(decltype(&C::fn_name));               \
    template <typename C>                                            \
    static std::false_type test(...);                                \
                                                                     \
   public:                                                           \
    static constexpr bool value = decltype(test<T>(nullptr))::value; \
  };                                                                 \
  template <typename T>                                              \
  static inline constexpr bool trait_name##_v = trait_name<T>::value

// Similar to DECLARE_HAS_MEMBER_FN but also checks the function signature.
// This is especially useful when the desired function may be overloaded.
// The signature must take the form "ResultType (C::*)(ArgType1, ArgType2)".
//
// Example:
//
// DECLARE_HAS_MEMBER_FN_WITH_SIGNATURE(has_c_str, c_str, const char* (C::*)() const);
#define DECLARE_HAS_MEMBER_FN_WITH_SIGNATURE(trait_name, fn_name, sig)   \
  template <typename T>                                                  \
  struct trait_name {                                                    \
   private:                                                              \
    template <typename C>                                                \
    static std::true_type test(decltype(static_cast<sig>(&C::fn_name))); \
    template <typename C>                                                \
    static std::false_type test(...);                                    \
                                                                         \
   public:                                                               \
    static constexpr bool value = decltype(test<T>(nullptr))::value;     \
  };                                                                     \
  template <typename T>                                                  \
  static inline constexpr bool trait_name##_v = trait_name<T>::value

// Similar to DECLARE_HAS_MEMBER_FN but for member types.
//
// Example:
//
// DECLARE_HAS_MEMBER_TYPE(has_value_type, ValueType);
#define DECLARE_HAS_MEMBER_TYPE(trait_name, type_name)               \
  template <typename T>                                              \
  struct trait_name {                                                \
   private:                                                          \
    template <typename C>                                            \
    static std::true_type test(typename C::type_name*);              \
    template <typename C>                                            \
    static std::false_type test(...);                                \
                                                                     \
   public:                                                           \
    static constexpr bool value = decltype(test<T>(nullptr))::value; \
  };                                                                 \
  template <typename T>                                              \
  static inline constexpr bool trait_name##_v = trait_name<T>::value

#endif  // FBL_MACROS_H_
