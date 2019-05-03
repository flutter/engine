// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_PICTURE_SHADER_H_
#define FLUTTER_LIB_UI_PAINTING_PICTURE_SHADER_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/shader.h"

namespace flutter {

class PictureShader : public Shader {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(PictureShader);

 public:
  ~PictureShader() override;
  static fml::RefPtr<PictureShader> Create();

 private:
  PictureShader();
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_PICTURE_SHADER_H_
