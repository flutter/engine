// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DL_SHARED_H_
#define FLUTTER_DISPLAY_LIST_DL_SHARED_H_

#include <cstddef>
#include <cstdint>

#include "fml/logging.h"

namespace flutter {

/// ----------------------------------------------------------------------
/// A template class to manage shared pointers to objects which implement
/// a compatible API, such as the DlShareable class defined below.
///
/// Assigning a pointer into a |dl_shared| instance will automatically
/// add a strong reference to the object that prevents it from being
/// disposed until the |dl_shared| instance is disposed, or its pointer
/// is replaced.
///
/// Access to the object is as if it was simply a pointer to the object
/// with the `*`, `->`, and `bool()` operations all acting as if they
/// were executed against the managed object pointer. In addition, a
/// raw, untracked pointer to the underlying object may be retreived using
/// the |get()| method, the lifetime of which should not exceed the lifetime
/// of that pointer remaining in the |dl_shared| instance.
///
/// A dl_shared instance is not meant to be accessed from multiple threads,
/// though separate dl_shared instances in different threads may all point
/// to the same underlying shareable object as long as its reference
/// management methods are themselves thread-safe (as they are in the
/// standard implementation of |DlShareable|.
template <typename T>
class dl_shared {
 public:
  dl_shared() : shareable_(nullptr) {}
  dl_shared(std::nullptr_t) : shareable_(nullptr) {}

  dl_shared(T* shareable) : shareable_(shareable) {  //
    shareable->AddStrongRef();
  }
  dl_shared(T& shareable) : dl_shared(&shareable) {}
  dl_shared(T&& shareable) : dl_shared(&shareable) {}

  ~dl_shared() {
    if (shareable_) {
      shareable_->RemoveStrongRef();
      shareable_ = nullptr;
    }
  }

  [[nodiscard]] T* release() {
    T* ret = shareable_;
    shareable_ = nullptr;
    return ret;
  }

  dl_shared<T>& operator=(dl_shared<T>&& that) {
    this->reset(that.release());
    return *this;
  }

  T& operator*() const {
    FML_DCHECK(shareable_ != nullptr);
    return *shareable_;
  }

  explicit operator bool() const { return shareable_ != nullptr; }

  T* get() const { return shareable_; }
  T* operator->() const { return shareable_; }

 private:
  T* shareable_;
};

/// ----------------------------------------------------------------------
/// A template class to manage weak shared pointers to objects which
/// implement a compatible API, such as the DlShareable class defined
/// below.
///
/// Assigning a pointer into a |dl_weak_shared| instance will automatically
/// add a weak reference to the object which does not indicate an active
/// interest in maintaining the object, but will nonetheless prevent its
/// destruction until all of both weak and strong shared references are
/// destroyed or replaced.
///
/// A dl_weak_shared instance is not meant to be accessed from multiple
/// threads, though separate dl_weak_shared instances in different threads
/// may all point to the same underlying shareable object as long as its
/// reference management methods are themselves thread-safe (as they are
/// in the standard implementation of |DlShareable|.
template <typename T>
class dl_weak_shared {
 public:
  dl_weak_shared() : shareable_(nullptr) {}
  dl_weak_shared(std::nullptr_t) : shareable_(nullptr) {}

  dl_weak_shared(T* shareable) : shareable_(shareable) {
    shareable->AddWeakRef();
  }
  dl_weak_shared(T& shareable) : dl_weak_shared(&shareable) {}
  dl_weak_shared(T&& shareable) : dl_weak_shared(&shareable) {}

  ~dl_weak_shared() {
    if (shareable_) {
      shareable_->RemoveWeakRef();
      shareable_ = nullptr;
    }
  }

  /// Returns true iff the underlying shareable object is weakly held.
  /// This property can be used during cache management to clean out
  /// references that no longer have a current compelling use.
  bool is_weakly_held() { return shareable_->is_weakly_held(); }

  /// The only access to the underlying object is to first strengthen
  /// the |dl_weak_shared| reference into a strong |dl_shared| reference
  /// using this method. At that point, dereferencing of the strong
  /// reference can commence as usual.
  ///
  /// The returned object will contain a strong reference to the
  /// shared object that is maintained independently to the weak
  /// reference held here.
  ///
  /// By default, this method will not strengthen a reference to an
  /// object that is referenced only from other weak references, but
  /// such a reference can be strengthened intentionally by passing
  /// in a true value for the |even_if_weak| argument.
  dl_shared<T> strengthen(bool even_if_weak = false) {
    if (!shareable_) {
      return nullptr;
    }
    if (!even_if_weak && shareable_->is_weakly_held()) {
      return nullptr;
    }
    return dl_shared<T>(shareable_);
  }

 private:
  T* shareable_;
};

/// ----------------------------------------------------------------------
/// The base class of any DisplayList object which wants to be
/// shareable using the |dl_shared<>| mechanism. This base includes
/// reference counts to manage strong and weak references to the
/// object. Weak references are not much different than strong
/// references other than being marked as not holding a strong
/// interest in keeping the object alive. The object will remain
/// alive until all references, both strong and weak, have been
/// revoked, though, so effort should be made to periodically
/// clean out any caches holding weak references to objects.
class DlShareable {
 public:
  uint32_t total_ref_count() { return total_ref_count_.load(); }
  uint32_t strong_ref_count() {
    // Internally weak references also add a strong reference
    // for lifetime management, but their contribution to the
    // weak ref count indicates the weakness of their interest
    // in the object itself.
    return total_ref_count_.load() - weak_ref_count_.load();
  }
  uint32_t weak_ref_count() { return weak_ref_count_.load(); }

  bool is_unique() { return total_ref_count_.load() == 1; }
  bool is_weakly_held() { return strong_ref_count() == 0; }

 protected:
  DlShareable() : total_ref_count_(1), weak_ref_count_(0) {}
  virtual ~DlShareable() {  //
    FML_DCHECK(total_ref_count_.load() == 0);
  }

  virtual void DlShareableDispose() const {  //
    delete this;
  }

 private:
  mutable std::atomic<uint32_t> total_ref_count_;
  mutable std::atomic<uint32_t> weak_ref_count_;

  void AddStrongRef() { total_ref_count_.fetch_add(1u); }
  void RemoveStrongRef() {
    FML_DCHECK(total_ref_count_ > 0);
    if (total_ref_count_.fetch_sub(1u) == 1) {
      DlShareableDispose();
    }
  }
  void AddWeakRef() {
    total_ref_count_.fetch_add(1u);
    weak_ref_count_.fetch_add(1u);
  }
  void RemoveWeakRef() {
    FML_DCHECK(weak_ref_count_.load() > 0);
    weak_ref_count_.fetch_sub(1u);
    RemoveStrongRef();
  }

  friend class dl_shared<DlShareable>;
  friend class dl_weak_shared<DlShareable>;
};

template <typename T, typename U>
inline bool operator==(const dl_shared<T>& a, const dl_shared<U>& b) {
  return a.get() == b.get();
}
template <typename T>
inline bool operator==(const dl_shared<T>& a, std::nullptr_t) {
  return !a;
}
template <typename T>
inline bool operator==(std::nullptr_t, const dl_shared<T>& b) {
  return !b;
}

template <typename T, typename U>
inline bool operator==(const dl_weak_shared<T>& a, const dl_weak_shared<U>& b) {
  return a.get() == b.get();
}
template <typename T>
inline bool operator==(const dl_weak_shared<T>& a, std::nullptr_t) {
  return !a;
}
template <typename T>
inline bool operator==(std::nullptr_t, const dl_weak_shared<T>& b) {
  return !b;
}

template <typename T, typename U>
inline bool operator==(const dl_shared<T>& a, const dl_weak_shared<U>& b) {
  return a.get() == b.get();
}
template <typename T, typename U>
inline bool operator==(const dl_weak_shared<T>& a, const dl_shared<U>& b) {
  return a.get() == b.get();
}

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DL_SHARED_H_
