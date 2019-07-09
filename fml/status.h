// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_STATUS_H_
#define FLUTTER_FML_STATUS_H_

#include <string_view>

namespace fml {

/// Google's canonical list of errors, should be in sync with:
/// https://github.com/googleapis/googleapis/blob/master/google/rpc/code.proto
enum class StatusCode : int {
  kOk = 0,
  kCancelled = 1,
  kUnknown = 2,
  kInvalidArgument = 3,
  kDeadlineExceeded = 4,
  kNotFound = 5,
  kAlreadyExists = 6,
  kPermissionDenied = 7,
  kResourceExhausted = 8,
  kFailedPrecondition = 9,
  kAborted = 10,
  kOutOfRange = 11,
  kUnimplemented = 12,
  kInternal = 13,
  kUnavailable = 14,
  kDataLoss = 15,
  kUnauthenticated = 16,
  kDoNotUseReservedForFutureExpansionUseDefaultInSwitchInstead_ = 20
};

/// Class that represents the resolution of the execution of a procedure.  This
/// is used similarly to how exceptions might be used, typically as the return
/// value to a synchronous procedure or an argument to an asynchronous callback.
class Status final {
 public:
  /// Creates an 'ok' status.
  Status();

  Status(fml::StatusCode code, std::string_view message);

  fml::StatusCode code() const;

  int raw_code() const;

  /// A noop that helps with static analysis tools if you decide to ignore an
  /// error.
  void IgnoreError() const;

  /// @return 'true' when the code is kOk.
  bool ok() const;

  std::string_view message() const;

 private:
  int code_;
  std::string_view message_;
};

inline Status::Status()
    : code_(static_cast<int>(fml::StatusCode::kOk)), message_() {}

inline Status::Status(fml::StatusCode code, std::string_view message)
    : code_(static_cast<int>(code)), message_(message) {}

inline fml::StatusCode Status::code() const {
  return static_cast<fml::StatusCode>(code_);
}

inline int Status::raw_code() const {
  return code_;
}

inline void Status::IgnoreError() const {
  // noop
}

inline bool Status::ok() const {
  return code_ == static_cast<int>(fml::StatusCode::kOk);
}

inline std::string_view Status::message() const {
  return message_;
}

}  // namespace fml

#endif  // FLUTTER_FML_SIZE_H_
