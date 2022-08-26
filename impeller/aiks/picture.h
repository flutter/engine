// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <deque>
#include <memory>

#include "flutter/fml/macros.h"
#include "impeller/aiks/aiks_context.h"
#include "impeller/aiks/image.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/entity_pass.h"

namespace impeller {

class Picture {
 public:
  Picture() = default;

  ~Picture() = default;

  Picture(Picture& p) : pass_(std::move(p.pass_)) {}

  Picture(Picture&& p) : pass_(std::move(p.pass_)) {}

  std::optional<Snapshot> Snapshot(AiksContext& context);

  std::shared_ptr<Image> ToImage(AiksContext& context, ISize size);

  void SetPass(std::unique_ptr<EntityPass> pass) { pass_ = std::move(pass); }

  const std::unique_ptr<EntityPass>& GetPass() const { return pass_; }

 private:
  std::optional<impeller::Snapshot> DoSnapshot(AiksContext& context, Rect rect);

  std::unique_ptr<EntityPass> pass_;
};

}  // namespace impeller
