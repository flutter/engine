// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_DART_VM_LIFECYCLE_H_
#define FLUTTER_RUNTIME_DART_VM_LIFECYCLE_H_

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/runtime/dart_vm.h"

namespace blink {

class DartVMLifecycleReference {
 public:
  FML_WARN_UNUSED_RESULT
  static DartVMLifecycleReference Create(
      Settings settings,
      fml::RefPtr<DartSnapshot> vm_snapshot = nullptr,
      fml::RefPtr<DartSnapshot> isolate_snapshot = nullptr,
      fml::RefPtr<DartSnapshot> shared_snapshot = nullptr);

  DartVMLifecycleReference(DartVMLifecycleReference&&);

  ~DartVMLifecycleReference();

  operator bool() const { return static_cast<bool>(vm_); }

  DartVM* operator->() {
    FML_DCHECK(vm_);
    return vm_.get();
  }

  DartVM* operator&() {
    FML_DCHECK(vm_);
    return vm_.get();
  }

 private:
  bool abandoned_ = false;
  std::shared_ptr<DartVM> vm_;

  DartVMLifecycleReference(std::shared_ptr<DartVM> vm);

  FML_DISALLOW_COPY_AND_ASSIGN(DartVMLifecycleReference);
};

class DartVMAccessReference {
 public:
  DartVMAccessReference(DartVMAccessReference&&);

  ~DartVMAccessReference();

  FML_WARN_UNUSED_RESULT
  static DartVMAccessReference Create();

  operator bool() const { return static_cast<bool>(vm_); }

  DartVM* operator->() {
    FML_DCHECK(vm_);
    return vm_.get();
  }

  DartVM* operator&() {
    FML_DCHECK(vm_);
    return vm_.get();
  }

 private:
  bool abandoned_ = false;
  std::shared_ptr<DartVM> vm_;

  DartVMAccessReference(std::shared_ptr<DartVM> vm);

  FML_DISALLOW_COPY_AND_ASSIGN(DartVMAccessReference);
};

}  // namespace blink

#endif  // FLUTTER_RUNTIME_DART_VM_LIFECYCLE_H_
