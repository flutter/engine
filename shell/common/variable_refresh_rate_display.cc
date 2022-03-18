// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/variable_refresh_rate_display.h"
#include "flutter/fml/logging.h"

namespace flutter {

VariableRefreshRateDisplay::VariableRefreshRateDisplay(
    DisplayId display_id,
    const std::weak_ptr<VariableRefreshRateReporter> refresh_rate_reporter)
    : Display(display_id,
              refresh_rate_reporter.lock() == nullptr
                  ? 0
                  : refresh_rate_reporter.lock()->GetRefreshRate()),
      refresh_rate_reporter_(refresh_rate_reporter) {}

VariableRefreshRateDisplay::VariableRefreshRateDisplay(
    const std::weak_ptr<VariableRefreshRateReporter> refresh_rate_reporter)
    : Display(refresh_rate_reporter.lock() == nullptr
                  ? 0
                  : refresh_rate_reporter.lock()->GetRefreshRate()),
      refresh_rate_reporter_(refresh_rate_reporter) {}

double VariableRefreshRateDisplay::GetRefreshRate() const {
  if (auto refresh_rate = refresh_rate_reporter_.lock()) {
    return refresh_rate->GetRefreshRate();
  }
  return 0;
}

}  // namespace flutter
