// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_shared.h"

namespace flutter {

// Normally exists just to ensure that the header can be cleanly imported.

#ifdef DL_SHAREABLE_STATS

int64_t DlShareable::total_made_ = 0;
int64_t DlShareable::total_disposed_ = 0;
void DlShareable::report_shareable_counts() {
  FML_DCHECK(total_disposed_ <= total_made_);
  if (total_disposed_ > 0) {
    // The FML_LOG mechanism is likely to be uninitialized while
    // we initialize some static DlShareable instances, so we wait
    // to report statistics until at least one instance is disposed.
    FML_LOG(ERROR) << "Currently " << (total_made_ - total_disposed_)
                   << " out of " << total_made_ << " alive";
  }
}

#endif  // DL_SHAREABLE_STATS

}  // namespace flutter
