// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_SYS_CPP_TESTING_REALM_BUILDER_H_
#define LIB_SYS_CPP_TESTING_REALM_BUILDER_H_

#include <memory>

namespace sys {

namespace testing {

class RealmBuilder {
 public:
  RealmBuilder() = default;
  void AddComponent();
};

}  // namespace testing

}  // namespace sys

#endif  // LIB_SYS_CPP_TESTING_REALM_BUILDER_H_
