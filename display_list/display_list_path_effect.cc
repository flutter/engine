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

  SkPathEffect::DashInfo info;
  if (SkPathEffect::DashType::kDash_DashType ==
      sk_path_effect->asADash(&info)) {
    auto dash_path_effect = std::make_shared<DlDashPathEffect>(
        info.fIntervals, info.fCount, info.fPhase);
    info.fIntervals = dash_path_effect->interval();
    sk_path_effect->asADash(&info);
    return dash_path_effect;
  }
  // If not dash path effect, we will use UnknownPathEffect to wrap it.
  return std::make_shared<DlUnknownPathEffect>(sk_ref_sp(sk_path_effect));
}

}  // namespace flutter
