// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <set>
#include <string>

#include "flutter/fml/macros.h"
#include "impeller/base/version.h"

namespace impeller {

class ProcTableGLES;

class DescriptionGLES {
 public:
  explicit DescriptionGLES(const ProcTableGLES& gl);

  ~DescriptionGLES();

  bool IsValid() const;

  bool IsES() const;

  std::string GetString() const;

  bool HasExtension(const std::string& ext) const;

  /// @brief      Returns whether GLES includes the debug extension.
  ///
  /// @warning    There are known devices (at the time of this writing, both
  ///             the Pixel 6A and Pixel Fold included) that report this as
  ///             `true`, but will fail at runtime when attempting to use the
  ///             functions (i.e. `glPushDebugGroup` and `glPopDebugGroup`).
  ///
  ///             Call `glDebugMessageControl` to setup the debug state before
  ///             using the debug functions, otherwise a `GL_OUT_OF_MEMORY`
  ///             error will be generated in debug mode.
  bool HasDebugExtension() const;

 private:
  Version gl_version_;
  Version sl_version_;
  bool is_es_ = true;
  std::string vendor_;
  std::string renderer_;
  std::string gl_version_string_;
  std::string sl_version_string_;
  std::set<std::string> extensions_;
  bool is_valid_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(DescriptionGLES);
};

}  // namespace impeller
