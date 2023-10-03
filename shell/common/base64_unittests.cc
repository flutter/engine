// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/base64.h"

#include "fml/logging.h"
#include "gtest/gtest.h"

#include <string>

namespace flutter {
namespace testing {

TEST(Base64, EncodeStrings) {
    auto test = [](std::string input, std::string output) {
        char buffer[256];
        size_t len = Base64::Encode(input.c_str(), input.length(), &buffer);
        FML_CHECK(len <= 256);
        std::string actual(buffer, len);
        ASSERT_STREQ(actual.c_str(), output.c_str());
    };
    test("apple", "YXBwbGU=");
    test("BANANA", "QkFOQU5B");
    test("Cherry Pie", "Q2hlcnJ5IFBpZQ==");
    test("fLoCcInAuCiNiHiLiPiLiFiCaTiOn", "ZkxvQ2NJbkF1Q2lOaUhpTGlQaUxpRmlDYVRpT24=");
    test("", "");
}

TEST(Base64, EncodeBytes) {
    auto test = [](const uint8_t input[], size_t num, std::string output) {
        char buffer[256];
        size_t len = Base64::Encode(input, num, &buffer);
        FML_CHECK(len <= 256);
        std::string actual(buffer, len);
        ASSERT_STREQ(actual.c_str(), output.c_str());
    };
    uint8_t buffer[] = {0x03, 0x14, 0x15, 0x92, 0x65};
    test(buffer, sizeof(buffer), "AxQVkmU=");
}

}  // namespace testing
}  // namespace flutter