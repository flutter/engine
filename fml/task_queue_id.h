// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_TASK_QUEUE_ID_H_
#define FLUTTER_FML_TASK_QUEUE_ID_H_

#include "flutter/fml/logging.h"

namespace fml {

class TaskQueueId {
 public:
  static const size_t kUnmerged;

  explicit TaskQueueId(size_t value) : value_(value) {}

  operator int() const { return value_; }

 private:
  size_t value_ = kUnmerged;
};

}  // namespace fml

#endif  // FLUTTER_FML_TASK_QUEUE_ID_H_
