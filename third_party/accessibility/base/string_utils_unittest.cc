// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "string_utils.h"

#include <cerrno>
#include <cstddef>
#include <string>

#include "base/logging.h"
#include "gtest/gtest.h"

namespace base {

TEST(StringUtilsTest, StringPrintfEmpty) {
  EXPECT_EQ("", base::StringPrintf("%s", ""));
}

TEST(StringUtilsTest, StringPrintfMisc) {
  EXPECT_EQ("123hello w", StringPrintf("%3d%2s %1c", 123, "hello", 'w'));
}
// Test that StringPrintf and StringAppendV do not change errno.
TEST(StringUtilsTest, StringPrintfErrno) {
  errno = 1;
  EXPECT_EQ("", StringPrintf("%s", ""));
  EXPECT_EQ(1, errno);
}

TEST(StringUtilsTest, canASCIIToUTF16) {
  std::string ascii = "abcdefg";
  EXPECT_EQ(ASCIIToUTF16(ascii).compare(u"abcdefg"), 0);
}

TEST(StringUtilsTest, canUTF8ToUTF16) {
  std::string utf8 = "äåè";
  EXPECT_EQ(UTF8ToUTF16(utf8).compare(u"äåè"), 0);
}

TEST(StringUtilsTest, canUTF16ToUTF8) {
  std::u16string utf16 = u"äåè";
  EXPECT_EQ(UTF16ToUTF8(utf16).compare("äåè"), 0);
}

TEST(StringUtilsTest, canNumberToString16) {
  float number = 1.123;
  EXPECT_EQ(NumberToString16(number), std::u16string(u"1.123"));
}

TEST(StringUtilsTest, numberToStringSimplifiesOutput) {
  double d0 = 0.0;
  EXPECT_STREQ(NumberToString(d0).c_str(), "0");
  float f0 = 0.0f;
  EXPECT_STREQ(NumberToString(f0).c_str(), "0");
  double d1 = 1.123;
  EXPECT_STREQ(NumberToString(d1).c_str(), "1.123");
  float f1 = 1.123f;
  EXPECT_STREQ(NumberToString(f1).c_str(), "1.123");
  double d2 = -1.123;
  EXPECT_STREQ(NumberToString(d2).c_str(), "-1.123");
  float f2 = -1.123f;
  EXPECT_STREQ(NumberToString(f2).c_str(), "-1.123");
  double d3 = 1.00001;
  EXPECT_STREQ(NumberToString(d3).c_str(), "1.00001");
  float f3 = 1.00001f;
  EXPECT_STREQ(NumberToString(f3).c_str(), "1.00001");
  double d4 = 1000.000001;
  EXPECT_STREQ(NumberToString(d4).c_str(), "1000.000001");
  float f4 = 10.00001f;
  EXPECT_STREQ(NumberToString(f4).c_str(), "10.00001");
  double d5 = 1.0 + 1e-8;
  EXPECT_STREQ(NumberToString(d5).c_str(), "1");
  float f5 = 1.0f + 1e-8f;
  EXPECT_STREQ(NumberToString(f5).c_str(), "1");
  double d6 = 1e-6;
  EXPECT_STREQ(NumberToString(d6).c_str(), "0.000001");
  float f6 = 1e-6f;
  // This is different from the double version because of
  // the precision difference in floats vs doubles.
  EXPECT_STREQ(NumberToString(f6).c_str(), "1e-6");
  double d7 = 1e-8;
  EXPECT_STREQ(NumberToString(d7).c_str(), "1e-8");
  float f7 = 1e-8f;
  EXPECT_STREQ(NumberToString(f7).c_str(), "1e-8");
  double d8 = 100.0;
  EXPECT_STREQ(NumberToString(d8).c_str(), "100");
  float f8 = 100.0f;
  EXPECT_STREQ(NumberToString(f8).c_str(), "100");
  double d9 = 1.0 + 1e-7;
  EXPECT_STREQ(NumberToString(d9, 7).c_str(), "1.0000001");
  float f9 = 1.0f + 1e-7f;
  EXPECT_STREQ(NumberToString(f9, 7).c_str(), "1.0000001");
  EXPECT_STREQ(NumberToString(d9, 0).c_str(), "1");
  EXPECT_STREQ(NumberToString(f9, 0).c_str(), "1");
  double d10 = 0.00000012345678;
  EXPECT_STREQ(NumberToString(d10, 6).c_str(), "1.234568e-7");
  float f10 = 0.00000012345678f;
  EXPECT_STREQ(NumberToString(f10, 6).c_str(), "1.234568e-7");
  unsigned int s = 11;
  EXPECT_STREQ(NumberToString(s).c_str(), "11");
  int32_t i = -23;
  EXPECT_STREQ(NumberToString(i).c_str(), "-23");
}

}  // namespace base
