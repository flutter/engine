// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COMPARABLE_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COMPARABLE_H_

#include <memory>

namespace flutter {

// These templates implement content comparisons that compare any two
// objects of the same type whether they are held as a raw pointer (T*),
// a reference (T&), or a shared pointer (std::shared_ptr<T>) (provided
// that the <T> class implements the == operator override).
// Any combination of pointers or references to T are supported and
// null pointers are not equal to anything but another null pointer.

namespace dl_comparable {

// Templates inside the dl_comparable namespace are considered private
// implementation details for the public-facing templates defined below
// just after the namespace

// -----------------------------------------------------------------------
// The ToPtr templates simply strip any of the supported reference types
// down into a raw pointer to their underlying object type.

template <typename T>
T* ToPtr(std::shared_ptr<T>& value) {
  return value.get();
}

template <typename T>
T* ToPtr(T* value) {
  return value;
}

template <typename T>
T* ToPtr(T& value) {
  return &value;
}

template <typename T>
T* ToPtr(std::nullptr_t) {
  return nullptr;
}

// -----------------------------------------------------------------------
// The EqualsHelper template performs the actual content comparison once
// we have established that there are two pointers to the same underlying
// object type. Pointer comparisons are used solely as an optimization
// under the assumption that A == A in the overloaded operator.

template <typename T>
bool Equals(const T* a, const T* b) {
  if (a == b) {
    return true;
  }
  if (!a || !b) {
    return false;
  }
  return *a == *b;
}

template <typename T,
          typename U,
          typename =
              typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
bool Equals(const T* a, const U* b) {
  if (a == b) {
    return true;
  }
  if (!a || !b) {
    return false;
  }
  // Let the convertible-from type manage the ==
  return *b == *a;
}

template <typename T,
          typename U,
          typename =
              typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
bool Equals(const U* a, const T* b) {
  if (a == b) {
    return true;
  }
  if (!a || !b) {
    return false;
  }
  // Let the convertible-from type manage the ==
  return *a == *b;
}

}  // namespace dl_comparable

// ------------------------------------------------------------------------
// The two Equals templates first break any pair of reference types down
// into raw pointers and then attempt to pass them to the internal Equals
// template, succeeding iff both underlying object types are identical.
//
// Thus Equals(T* a, shared_ptr<const T> b) compiles (if T implements == T)
// but  Equals(T* a, U* b)                  (U related to T) compiles
// but  Equals(T* a, V* b)                  (V unrelated to T) does not.

template <typename T, typename U>
bool Equals(T a, U b) {
  return dl_comparable::Equals(dl_comparable::ToPtr(a),
                               dl_comparable::ToPtr(b));
}

template <typename T, typename U>
bool NotEquals(T a, U b) {
  return !dl_comparable::Equals(dl_comparable::ToPtr(a),
                                dl_comparable::ToPtr(b));
}

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_COMPARABLE_H_
