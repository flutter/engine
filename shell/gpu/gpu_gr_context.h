
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_GPU_GPU_GR_CONTEXT_H_
#define SHELL_GPU_GPU_GR_CONTEXT_H_

#include "third_party/skia/include/gpu/GrContext.h"

namespace shell {
  
class GpuGrContext {
public:
    GpuGrContext(bool createContext);
    ~GpuGrContext();
    
    // Pass createContext=false for software rendering.
    GrContext *GetContext();
private:
    sk_sp<GrContext> context_;
};

} // namespace shell

#endif // SHELL_GPU_GPU_GR_CONTEXT_H_
