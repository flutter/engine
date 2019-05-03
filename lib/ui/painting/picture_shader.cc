// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/picture_shader.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, PictureShader);

fml::RefPtr<PictureShader> PictureShader::Create() {
  return fml::MakeRefCounted<PictureShader>();
}

PictureShader::PictureShader() = default;

PictureShader::~PictureShader() = default;

}  // namespace flutter
