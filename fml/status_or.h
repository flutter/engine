// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_STATUS_OR_H_
#define FLUTTER_FML_STATUS_OR_H_

#include <optional>

#include "flutter/fml/status.h"

namespace fml {

/// TODO(): Replace with absl::StatusOr.
template <typename T>
class StatusOr {
 public:
  StatusOr(const T& value) : status_(), value_(value) {}
  StatusOr(const Status& status) : status_(status), value_() {}

  StatusOr(const StatusOr&) = default;
  StatusOr(StatusOr&&) = default;

  StatusOr& operator=(const StatusOr&) = default;
  StatusOr& operator=(StatusOr&&) = default;

  StatusOr& operator=(const T& value) {
    status_ = Status();
    value_ = value;
    return *this;
  }

  StatusOr& operator=(const T&& value) {
    status_ = Status();
    value_ = std::move(value);
    return *this;
  }

  StatusOr& operator=(const Status& value) {
    status_ = value;
    value_ = std::nullopt;
    return *this;
  }

  const Status& status() const { return status_; }

  bool ok() const { return status_.ok(); }

  const T& value() const {
    FML_CHECK(status_.ok());
    return value_.value();
  }

  T& value() {
    FML_CHECK(status_.ok());
    return value_.value();
  }

 private:
  Status status_;
  std::optional<T> value_;
};

}  // namespace fml

#endif
