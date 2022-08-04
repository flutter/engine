// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_THREADSAFE_UNIQUE_PTR_H_
#define FLUTTER_FML_THREADSAFE_UNIQUE_PTR_H_

#include <memory>
#include <mutex>

#include "flutter/fml/macros.h"

namespace fml {

/// A single-owner smart pointer that allows weak references that can be
/// accessed on different threads.
/// This smart-pointer makes the following guarantees:
///   * There is only ever one threadsafe_unique_ptr per object.
///   * The object is deleted when ~threadsafe_unique_ptr() is called.
///   * The object will not be deleted whilst another thread has a lock_ptr.
///   * The thread that owns the object can access it without a lock.
/// WARNING: weak_ptr's should only be used to invoke thread-safe methods.
template <typename T>
class threadsafe_unique_ptr {
 private:
  struct Data {
    Data(std::unique_ptr<T>&& arg_ptr) : ptr(std::move(arg_ptr)) {}
    std::unique_ptr<T> ptr;
    std::mutex mutex;
  };

 public:
  threadsafe_unique_ptr()
      : data_(new Data(std::unique_ptr<T>())), fast_ptr_(nullptr) {}

  threadsafe_unique_ptr(std::unique_ptr<T>&& ptr)
      : data_(new Data(std::move(ptr))) {
    fast_ptr_ = data_->ptr.get();
  }

  threadsafe_unique_ptr(threadsafe_unique_ptr<T>&& ptr)
      : data_(std::move(ptr.data_)), fast_ptr_(std::move(ptr.fast_ptr_)) {}

  threadsafe_unique_ptr& operator=(threadsafe_unique_ptr&& rvalue) {
    data_ = std::move(rvalue.data_);
    fast_ptr_ = std::move(rvalue.fast_ptr_);
    return *this;
  }

  ~threadsafe_unique_ptr() {
    if (data_) {
      std::scoped_lock lock(data_->mutex);
      data_->ptr.reset();
    }
  }

  T* operator->() const { return fast_ptr_; }

  T* get() const { return fast_ptr_; }

  operator bool() const { return fast_ptr_ != nullptr; }

  class lock_ptr;

  /// A non-owning smart pointer for a `threadsafe_unique_ptr`.
  class weak_ptr {
   public:
    weak_ptr() {}

    weak_ptr(std::weak_ptr<Data> weak_ptr) : weak_ptr_(weak_ptr) {}

   private:
    friend class lock_ptr;
    std::weak_ptr<Data> weak_ptr_;
  };

  /// A temporary owning smart pointer for a `threadsafe_unique_ptr`.  This
  /// guarantees that the object will not be deleted while in scope.
  class lock_ptr {
   public:
    lock_ptr(const weak_ptr& weak) : strong_ptr_(weak.weak_ptr_.lock()) {
      if (strong_ptr_) {
        lock_ = std::unique_lock(strong_ptr_->mutex);
      }
    }

    T* operator->() { return strong_ptr_ ? strong_ptr_->ptr.get() : nullptr; }

    operator bool() const {
      return strong_ptr_ ? static_cast<bool>(strong_ptr_->ptr) : false;
    }

   private:
    FML_DISALLOW_COPY_AND_ASSIGN(lock_ptr);
    std::shared_ptr<Data> strong_ptr_;
    std::unique_lock<std::mutex> lock_;
  };

  weak_ptr GetWeakPtr() const { return weak_ptr(data_); }

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(threadsafe_unique_ptr);
  std::shared_ptr<Data> data_;
  // Clients that own the threadsafe_unique_ptr can access the pointer directly
  // since there is no risk that it will be deleted whilst being accessed.
  T* fast_ptr_;
};

}  // namespace fml

#endif  // FLUTTER_FML_THREADSAFE_UNIQUE_PTR_H_
