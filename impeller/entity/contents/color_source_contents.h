// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/geometry/matrix.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/vertices.h"

namespace impeller {

class ColorSourceContents : public Contents {
 public:
  ColorSourceContents();

  ~ColorSourceContents() override;

  void SetPath(Path path);

  void SetVertices(Vertices vertices);

  void SetMatrix(Matrix matrix);

  void SetAlpha(Scalar alpha);

  void SetCover(bool cover);

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  bool ShouldRender(const Entity& entity,
                    const std::optional<Rect>& stencil_coverage) const override;

 protected:
  const std::optional<Path>& GetPath() const;

  const std::optional<Vertices>& GetVertices() const;

  const Matrix& GetInverseMatrix() const;

  Scalar GetAlpha() const;

  bool GetCover() const;

 private:
  std::optional<Path> path_;
  std::optional<Vertices> vertices_;
  Matrix inverse_matrix_;
  Scalar alpha_ = 1.0;
  bool cover_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(ColorSourceContents);
};

}  // namespace impeller
