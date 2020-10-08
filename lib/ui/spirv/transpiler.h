// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SSIR_TRANSPILER_TRANSPILER_H_
#define SSIR_TRANSPILER_TRANSPILER_H_

#include <memory>
#include <string>

namespace flutter {
namespace spirv {

// Error codes for transpiling.
enum Status {
  kSuccess = 0,
  kFailedToInitialize = 1,
  kInvalidData = 2,
  kFailure = 3,
};

// Outcome of an Interpret call.
struct Result {
  Status status;
  std::string message;
};

// Transpiler for SSIR to SkSL.
class Transpiler {
 public:
  static std::unique_ptr<Transpiler> create();

  virtual ~Transpiler() = default;

  virtual Result Transpile(const char* data, size_t length) = 0;

  virtual std::string GetSkSL() = 0;

 protected:
  Transpiler() = default;
};

}  // namespace spirv
}  // namespace flutter

#endif  // SSIR_TRANSPILER_TRANSPILER_H_
