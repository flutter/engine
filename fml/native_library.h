// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_NATIVE_LIBRARY_H_
#define FLUTTER_FML_NATIVE_LIBRARY_H_

#include "lib/fxl/macros.h"
#include "lib/fxl/memory/ref_counted.h"
#include "lib/fxl/memory/ref_ptr.h"

namespace fml {
class NativeLibrary : public fxl::RefCountedThreadSafe<NativeLibrary> {
 public:
  static fxl::RefPtr<NativeLibrary> Create(const char* path);

  static fxl::RefPtr<NativeLibrary> CreateForCurrentProcess();

  const uint8_t* ResolveSymbol(const char* symbol);

 private:
  void* handle_ = nullptr;
  bool close_handle_ = true;

  NativeLibrary(const char* path);

  NativeLibrary(void* handle, bool close_handle);

  ~NativeLibrary();

  void* GetHandle() const;

  FXL_DISALLOW_COPY_AND_ASSIGN(NativeLibrary);
  FRIEND_REF_COUNTED_THREAD_SAFE(NativeLibrary);
};

}  // namespace fml

#endif  // FLUTTER_FML_NATIVE_LIBRARY_H_
