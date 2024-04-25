// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/toolkit/wasm/context.h"

#include "impeller/base/validation.h"

namespace impeller::wasm {

Context::Context() {
  if (!display_.IsValid()) {
    VALIDATION_LOG << "Could not create EGL display connection.";
    return;
  }

  egl::ConfigDescriptor desc;
  desc.api = egl::API::kOpenGLES2;
  desc.samples = egl::Samples::kOne;
  desc.color_format = egl::ColorFormat::kRGBA8888;
  desc.stencil_bits = egl::StencilBits::kZero;
  desc.depth_bits = egl::DepthBits::kZero;
  desc.surface_type = egl::SurfaceType::kWindow;

  auto config = display_.ChooseConfig(desc);
  if (!config) {
    VALIDATION_LOG << "Could not choose an EGL config.";
    return;
  }

  auto context = display_.CreateContext(*config, nullptr);
  if (!context) {
    VALIDATION_LOG << "Could not create EGL context.";
    return;
  }

  // The native window type is NULL for emscripten as documented in
  // https://emscripten.org/docs/porting/multimedia_and_graphics/EGL-Support-in-Emscripten.html
  auto surface = display_.CreateWindowSurface(*config, NULL);
  if (!surface) {
    VALIDATION_LOG << "Could not create EGL surface.";
    return;
  }

  surface_ = std::move(surface);
  context_ = std::move(context);
  is_valid_ = true;
}

Context::~Context() = default;

bool Context::IsValid() const {
  return is_valid_;
}

bool Context::RenderFrame() {
  if (!IsValid()) {
    return false;
  }

  if (!context_->MakeCurrent(*surface_)) {
    return false;
  }

  return true;
}

}  // namespace impeller::wasm
