// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform/mojo/dart_tracing_impl.h"

#include <utility>

#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/cpp/bindings/interface_handle.h"
#include "mojo/services/tracing/interfaces/tracing.mojom.h"
#include "mojo/services/tracing/interfaces/trace_provider_registry.mojom.h"

namespace sky {
namespace shell {

DartTracingImpl::DartTracingImpl() {
}

DartTracingImpl::~DartTracingImpl() {
}

void DartTracingImpl::Initialize(mojo::ApplicationImpl* app) {
  tracing::TraceProviderRegistryPtr registry;
  ConnectToService(app->shell(), "mojo:tracing", GetProxy(&registry));
  mojo::InterfaceHandle<tracing::TraceProvider> provider;
  provider_impl_.Bind(GetProxy(&provider));
  registry->RegisterTraceProvider(provider.Pass());
}

}  // namespace shell
}  // namespace sky
