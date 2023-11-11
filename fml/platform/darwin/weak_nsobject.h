// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_DARWIN_WEAK_NSOBJECT_H_
#define FLUTTER_FML_PLATFORM_DARWIN_WEAK_NSOBJECT_H_

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

#include <stdlib.h>
#include "flutter/fml/compiler_specific.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/memory/thread_checker.h"

namespace debug {
struct DebugThreadChecker {
  FML_DECLARE_THREAD_CHECKER(checker);
};
}  // namespace debug

// WeakNSObject<> is patterned after scoped_nsobject<>, but instead of
// maintaining ownership of an NSObject subclass object, it will nil itself out
// when the object is deallocated.
//
// WeakNSProtocol<> has the same behavior as WeakNSObject, but can be used
// with protocols.
//
// Example usage (fml::WeakNSObject<T>):
//   WeakNSObjectFactory factory([[Foo alloc] init]);
//   WeakNSObject<Foo> weak_foo;  // No pointer
//   weak_foo = factory.GetWeakNSObject() // Now a weak reference is kept.
//   [weak_foo description];  // Returns [foo description].
//   foo.reset();  // The reference is released.
//   [weak_foo description];  // Returns nil, as weak_foo is pointing to nil.
//
//
// Implementation wise a WeakNSObject keeps a reference to a refcounted
// WeakContainer. There is one unique instance of a WeakContainer per watched
// NSObject, this relationship is maintained via the ObjectiveC associated
// object API, indirectly via an ObjectiveC CRBWeakNSProtocolSentinel class.
//
// Threading restrictions:
// - Several WeakNSObject pointing to the same underlying object must all be
//   created and dereferenced on the same thread;
// - thread safety is enforced by the implementation, except:
//     - it is allowed to destroy a WeakNSObject on any thread;
// - the implementation assumes that the tracked object will be released on the
//   same thread that the WeakNSObject is created on.
//
// fml specifics:
// WeakNSObjects can only originate from a |WeakNSObjectFactory| (see below), though WeakNSObjects
// are copyable and movable.
//
// WeakNSObjects are not in general thread-safe. They may only be *used* on
// a single thread, namely the same thread as the "originating"
// |WeakNSObjectFactory| (which can invalidate the WeakNSObjects that it
// generates).
namespace fml {

// Forward declaration, so |WeakNSObject<NST>| can friend it.
template <typename NST>
class WeakNSObjectFactory;

// WeakContainer keeps a weak pointer to an object and clears it when it
// receives nullify() from the object's sentinel.
class WeakContainer : public fml::RefCountedThreadSafe<WeakContainer> {
 public:
  explicit WeakContainer(id object, std::shared_ptr<debug::DebugThreadChecker> checker = nullptr)
      : checker_(checker), object_(object) {}

  id object() {
    CheckThreadSafety();
    return object_;
  }

  void nullify() {
    CheckThreadSafety();
    object_ = nil;
  }

 private:
  friend fml::RefCountedThreadSafe<WeakContainer>;
  ~WeakContainer() {}

  std::shared_ptr<debug::DebugThreadChecker> checker_;
  id object_;

  void CheckThreadSafety() const {
    if (!checker_) {
      return;
    }
    FML_DCHECK_CREATION_THREAD_IS_CURRENT(checker_->checker);
  }
};

}  // namespace fml

// Sentinel for observing the object contained in the weak pointer. The object
// will be deleted when the weak object is deleted and will notify its
// container.
@interface CRBWeakNSProtocolSentinel : NSObject
// Return the only associated container for this object. There can be only one.
// Will return null if object is nil .
+ (fml::RefPtr<fml::WeakContainer>)containerForObject:(id)object
                                        threadChecker:
                                            (std::shared_ptr<debug::DebugThreadChecker>)checker;
@end

namespace fml {

// Base class for all WeakNSObject derivatives.
template <typename NST>
class WeakNSProtocol {
 public:
  WeakNSProtocol() = default;

  WeakNSProtocol(const WeakNSProtocol<NST>& that) {
    // A WeakNSProtocol object can be copied on one thread and used on
    // another.
    container_ = that.container_;
  }

  ~WeakNSProtocol() = default;

  void reset() {
    CheckThreadSafety();
    container_ = [CRBWeakNSProtocolSentinel containerForObject:nil threadChecker:checker_];
  }

  NST get() const {
    CheckThreadSafety();
    if (!container_.get()) {
      return nil;
    }
    return container_->object();
  }

  WeakNSProtocol& operator=(const WeakNSProtocol<NST>& that) {
    // A WeakNSProtocol object can be copied on one thread and used on
    // another.
    container_ = that.container_;
    return *this;
  }

  bool operator==(NST that) const {
    CheckThreadSafety();
    return get() == that;
  }

  bool operator!=(NST that) const {
    CheckThreadSafety();
    return get() != that;
  }

  operator NST() const {
    CheckThreadSafety();
    return get();
  }

 protected:
  friend class WeakNSObjectFactory<NST>;

  explicit WeakNSProtocol(std::shared_ptr<debug::DebugThreadChecker> checker, NST object = nil)
      : checker_(checker) {
    container_ = [CRBWeakNSProtocolSentinel containerForObject:object threadChecker:checker];
  }

  // Refecounted reference to the container tracking the ObjectiveC object this
  // class encapsulates.
  RefPtr<fml::WeakContainer> container_;
  std::shared_ptr<debug::DebugThreadChecker> checker_;

  void CheckThreadSafety() const {
    if (!checker_) {
      return;
    }
    FML_DCHECK_CREATION_THREAD_IS_CURRENT(checker_->checker);
  }
};

// Free functions
template <class NST>
bool operator==(NST p1, const WeakNSProtocol<NST>& p2) {
  return p1 == p2.get();
}

template <class NST>
bool operator!=(NST p1, const WeakNSProtocol<NST>& p2) {
  return p1 != p2.get();
}

template <typename NST>
class WeakNSObject : public WeakNSProtocol<NST*> {
 public:
  WeakNSObject() = default;
  WeakNSObject(const WeakNSObject<NST>& that) : WeakNSProtocol<NST*>(that) {}

  WeakNSObject& operator=(const WeakNSObject<NST>& that) {
    WeakNSProtocol<NST*>::operator=(that);
    return *this;
  }

 private:
  friend class WeakNSObjectFactory<NST>;

  explicit WeakNSObject(std::shared_ptr<debug::DebugThreadChecker> checker, NST* object = nil)
      : WeakNSProtocol<NST*>(checker, object) {}
};

// Specialization to make WeakNSObject<id> work.
template <>
class WeakNSObject<id> : public WeakNSProtocol<id> {
 public:
  WeakNSObject() = default;
  WeakNSObject(const WeakNSObject<id>& that) : WeakNSProtocol<id>(that) {}

  WeakNSObject& operator=(const WeakNSObject<id>& that) {
    WeakNSProtocol<id>::operator=(that);
    return *this;
  }

 private:
  friend class WeakNSObjectFactory<id>;

  explicit WeakNSObject(std::shared_ptr<debug::DebugThreadChecker> checker, id object = nil)
      : WeakNSProtocol<id>(checker, object) {}
};

// Class that produces (valid) |WeakNSObject<NST>|s. Typically, this is used as a
// member variable of |NST| (preferably the last one -- see below), and |NST|'s
// methods control how WeakNSObjects to it are vended. This class is not
// thread-safe, and should only be created, destroyed and used on a single
// thread.
//
// Example:
//
// ```objc
// @implementation Controller {
//  std::unique_ptr<fml::WeakNSObjectFactory<Controller>> _weakFactory;
// }
//
// - (instancetype)init {
//   self = [super init];
//   _weakFactory = std::make_unique<fml::WeakNSObjectFactory<Controller>>(self)
// }

// - (fml::WeakNSObject<Controller>) {
//   return _weakFactory->GetWeakNSObject()
// }
//
// @end
// ```
template <typename NST>
class WeakNSObjectFactory {
 public:
  explicit WeakNSObjectFactory(NST* object) : object_(object) {
    FML_DCHECK(object_);
    checker_ = std::make_shared<debug::DebugThreadChecker>();
  }

  ~WeakNSObjectFactory() { CheckThreadSafety(); }

  // Gets a new weak pointer, which will be valid until this object is
  // destroyed.
  WeakNSObject<NST> GetWeakNSObject() const { return WeakNSObject<NST>(checker_, object_); }

 private:
  NST* object_;

  void CheckThreadSafety() const {
    if (!checker_) {
      return;
    }
    FML_DCHECK_CREATION_THREAD_IS_CURRENT(checker_->checker);
  }

  std::shared_ptr<debug::DebugThreadChecker> checker_;

  FML_DISALLOW_COPY_AND_ASSIGN(WeakNSObjectFactory);
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_DARWIN_WEAK_NSOBJECT_H_
