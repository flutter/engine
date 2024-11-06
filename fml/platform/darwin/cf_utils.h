// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_DARWIN_CF_UTILS_H_
#define FLUTTER_FML_PLATFORM_DARWIN_CF_UTILS_H_

#include <CoreFoundation/CoreFoundation.h>

#include "flutter/fml/macros.h"

namespace fml {

/// RAII-based smart pointer wrapper for CoreFoundation objects.
///
/// CFRef takes over ownership of the object it wraps and ensures that CFRetain
/// and CFRelease are called as appropriate on creation, assignment, and
/// disposal.
template <class T>
class CFRef {
 public:
  /// Creates a new null CFRef.
  CFRef() : instance_(nullptr) {}

  /// Takes over ownership of `instance`, which is expected to be already
  /// retained.
  ///
  // NOLINTNEXTLINE(google-explicit-constructor)
  CFRef(T instance) : instance_(instance) {}

  /// Copy ctor: Creates a retained copy of the CoreFoundation object owned by
  /// `other`.
  CFRef(const CFRef& other) : instance_(other.instance_) {
    if (instance_) {
      CFRetain(instance_);
    }
  }

  /// Move ctor: Takes over ownership of the CoreFoundation object owned
  /// by `other`. The object owned by `other` is set to null.
  CFRef(CFRef&& other) : instance_(other.instance_) {
    other.instance_ = nullptr;
  }

  /// Takes over ownership of the CoreFoundation object owned by `other`.
  CFRef& operator=(CFRef&& other) {
    Reset(other.Release());
    return *this;
  }

  /// Releases the underlying CoreFoundation object, if non-null.
  ~CFRef() {
    if (instance_) {
      CFRelease(instance_);
    }
    instance_ = nullptr;
  }

  /// Takes over ownership of `instance`, null by default. The object is
  /// expected to be already retained if non-null.
  ///
  /// Releases the previous object, if non-null.
  void Reset(T instance = nullptr) {
    if (instance_) {
      CFRelease(instance_);
    }
    instance_ = instance;
  }

  /// Retains a shared copy of `instance`. The previous object is released if
  /// non-null. Has no effect if `instance` is the currently-held object.
  void Retain(T instance = nullptr) {
    if (instance_ == instance) {
      return;
    }
    if (instance) {
      CFRetain(instance);
    }
    Reset(instance);
  }

  /// Returns and transfers ownership of the underlying CoreFoundation object
  /// to the caller. The caller is responsible for calling `CFRelease` when done
  /// with the object.
  [[nodiscard]] T Release() {
    auto instance = instance_;
    instance_ = nullptr;
    return instance;
  }

  /// Returns the underlying CoreFoundation object. Ownership of the returned
  /// object follows The Get Rule.
  ///
  /// See:
  /// https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFMemoryMgmt/Concepts/Ownership.html#//apple_ref/doc/uid/20001148-SW1
  T Get() const { return instance_; }

  /// Returns the underlying CoreFoundation object. Ownership of the returned
  /// object follows The Get Rule.
  ///
  /// See:
  /// https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFMemoryMgmt/Concepts/Ownership.html#//apple_ref/doc/uid/20001148-SW1
  // NOLINTNEXTLINE(google-explicit-constructor)
  operator T() const { return instance_; }

  /// Returns true if the underlying CoreFoundation object is non-null.
  explicit operator bool() const { return instance_ != nullptr; }

 private:
  T instance_;

  CFRef& operator=(const CFRef&) = delete;
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_DARWIN_CF_UTILS_H_
