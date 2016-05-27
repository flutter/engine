// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_PLATFORM_MOJO_DART_TRACING_IMPL_H_
#define SKY_SHELL_PLATFORM_MOJO_DART_TRACING_IMPL_H_

#include "base/macros.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "sky/shell/platform/mojo/dart_trace_provider.h"

namespace sky {
namespace shell {

class DartTracingImpl {
 public:
  DartTracingImpl();
  ~DartTracingImpl();

  // This connects to the tracing service and registers ourselves to provide
  // tracing data on demand.
  void Initialize(mojo::ApplicationImpl* app);

 private:
  DartTraceProvider provider_impl_;

  DISALLOW_COPY_AND_ASSIGN(DartTracingImpl);
};

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_PLATFORM_MOJO_DART_TRACING_IMPL_H_
