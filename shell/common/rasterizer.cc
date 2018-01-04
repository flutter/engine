// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/rasterizer.h"

namespace shell {

Rasterizer::Rasterizer(blink::TaskRunners task_runners)
    : task_runners_(std::move(task_runners)), weak_factory_(this) {
  weak_prototype_ = weak_factory_.GetWeakPtr();
}

Rasterizer::~Rasterizer() = default;

fml::WeakPtr<Rasterizer> Rasterizer::GetWeakPtr() const {
  return weak_prototype_;
}

}  // namespace shell
