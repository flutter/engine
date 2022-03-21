// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_path_effect.h"
#include <memory>
#include "include/core/SkRefCnt.h"

namespace flutter {

std::shared_ptr<DlPathEffect> DlPathEffect::MakeSum(
    std::shared_ptr<DlPathEffect> first,
    std::shared_ptr<DlPathEffect> second) {
  return std::make_shared<DlSumPathEffect>(first, second);
}

std::shared_ptr<DlPathEffect> DlPathEffect::MakeCompose(
    std::shared_ptr<DlPathEffect> outer,
    std::shared_ptr<DlPathEffect> inner) {
  return std::make_shared<DlComposePathEffect>(outer, inner);
}

}  // namespace flutter
