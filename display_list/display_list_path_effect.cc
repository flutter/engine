// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_path_effect.h"
#include <memory>
#include "include/core/SkRefCnt.h"

namespace flutter {

std::shared_ptr<DlPathEffect> DlPathEffect::From(SkPathEffect* sk_path_effect) {
  if (sk_path_effect == nullptr) {
    return nullptr;
  }
  // There are no inspection methods for SkPathEffect so we cannot break
  // the Skia filter down into a specific subclass (i.e. DlSumPathEffect or
  // DlComposePathEffect).
  return std::make_shared<DlUnknownPathEffect>(sk_ref_sp(sk_path_effect));
}

std::shared_ptr<DlPathEffect> DlPathEffect::MakeSum(
    std::shared_ptr<DlPathEffect> first,
    std::shared_ptr<DlPathEffect> second) {
  return std::make_shared<DlSumPathEffect>(first->skia_object(),
                                           second->skia_object());
}

std::shared_ptr<DlPathEffect> DlPathEffect::MakeCompose(
    std::shared_ptr<DlPathEffect> outer,
    std::shared_ptr<DlPathEffect> inner) {
  return std::make_shared<DlComposePathEffect>(outer->skia_object(),
                                               inner->skia_object());
}

}  // namespace flutter
