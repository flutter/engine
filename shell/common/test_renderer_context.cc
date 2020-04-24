// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "test_renderer_context.h"

namespace flutter {
namespace testing {

int TestRendererContext::currentContext;

TestRendererContext::TestRendererContext(int context) : context_(context){};

TestRendererContext::~TestRendererContext() = default;

bool TestRendererContext::SetCurrent() {
  SetCurrentContext(context_);
  return true;
};

void TestRendererContext::RemoveCurrent() {
  SetCurrentContext(-1);
};

int TestRendererContext::GetContext() {
  return context_;
};

void TestRendererContext::SetCurrentContext(int context) {
  currentContext = context;
};


}
}
