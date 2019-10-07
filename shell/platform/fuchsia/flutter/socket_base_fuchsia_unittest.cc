// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <dart/runtime/bin/socket_base.h>
#include <gtest/gtest.h>

namespace flutter_runner_test::socket_base_test {

TEST(SocketBaseTest, TestGetInterfaces) {
  dart::bin::SocketBase::Initialize();
  dart::bin::OSError* os_error = NULL;
  const int type_any = dart::bin::SocketAddress::TYPE_ANY;
  auto interfaces = dart::bin::SocketBase::ListInterfaces(type_any, &os_error);
  ASSERT_FALSE(os_error);
  ASSERT_TRUE(interfaces->count() > 0);
}

}  // namespace flutter_runner_test::socket_base_test
