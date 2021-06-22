// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_SYS_CPP_TESTING_EXAMPLES_TEST_WITH_ENVIRONMENT_FAKE_ECHO_H_
#define LIB_SYS_CPP_TESTING_EXAMPLES_TEST_WITH_ENVIRONMENT_FAKE_ECHO_H_

#include <test/placeholders/cpp/fidl.h>
#include <lib/fidl/cpp/binding_set.h>

namespace echo {
namespace testing {

using test::placeholders::Echo;

// Fake server, which the client under test will be used against
class FakeEcho : public Echo {
 public:
  fidl::InterfaceRequestHandler<Echo> GetHandler() { return bindings_.GetHandler(this); }

  // Fake implementation of server-side logic
  void EchoString(fidl::StringPtr value, EchoStringCallback callback) { callback(answer_); }

  void SetAnswer(fidl::StringPtr answer) { answer_ = answer; }

 private:
  fidl::BindingSet<Echo> bindings_;
  fidl::StringPtr answer_;
};

}  // namespace testing
}  // namespace echo

#endif  // LIB_SYS_CPP_TESTING_EXAMPLES_TEST_WITH_ENVIRONMENT_FAKE_ECHO_H_
