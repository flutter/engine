// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_PLATFORM_MOJO_DART_TRACE_PROVIDER_H_
#define SKY_SHELL_PLATFORM_MOJO_DART_TRACE_PROVIDER_H_

#include "base/macros.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/interface_handle.h"
#include "mojo/public/cpp/bindings/interface_request.h"
#include "mojo/services/tracing/interfaces/tracing.mojom.h"

namespace sky {
namespace shell {

class DartTraceProvider : public tracing::TraceProvider {
 public:
  DartTraceProvider();
  ~DartTraceProvider() override;

  void Bind(mojo::InterfaceRequest<tracing::TraceProvider> request);

 private:
  // tracing::TraceProvider implementation:
  void StartTracing(
      const mojo::String& categories,
      mojo::InterfaceHandle<tracing::TraceRecorder> recorder) override;
  void StopTracing() override;

  void SplitAndRecord(char* data, size_t length);

  mojo::Binding<tracing::TraceProvider> binding_;
  tracing::TraceRecorderPtr recorder_;

  DISALLOW_COPY_AND_ASSIGN(DartTraceProvider);
};

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_PLATFORM_MOJO_DART_TRACE_PROVIDER_H_
