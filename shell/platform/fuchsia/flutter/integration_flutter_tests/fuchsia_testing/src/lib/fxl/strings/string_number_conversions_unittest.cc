// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/string_number_conversions.h"

#include <stdint.h>

#include <limits>

#include <gtest/gtest.h>

namespace fxl {
namespace {

TEST(StringNumberConversionsTest, NumberToString_Basic) {
  EXPECT_EQ("0", NumberToString<int32_t>(static_cast<int32_t>(0)));
  EXPECT_EQ("123", NumberToString<int32_t>(static_cast<int32_t>(123)));
  EXPECT_EQ("-456", NumberToString<int32_t>(static_cast<int32_t>(-456)));

  EXPECT_EQ("0", NumberToString<uint32_t>(static_cast<uint32_t>(0)));
  EXPECT_EQ("123", NumberToString<uint32_t>(static_cast<uint32_t>(123)));
  EXPECT_EQ("2309737967", NumberToString<int64_t>(static_cast<int64_t>(2309737967)));
  EXPECT_EQ("-2309737967", NumberToString<int64_t>(static_cast<int64_t>(-2309737967ll)));

  EXPECT_EQ("0", NumberToString<int>(0));
  EXPECT_EQ("123", NumberToString<int>(123));
  EXPECT_EQ("-456", NumberToString<int>(-456));

  EXPECT_EQ("0", NumberToString<unsigned>(0u));
  EXPECT_EQ("123", NumberToString<unsigned>(123u));

  EXPECT_EQ("1", NumberToString<int>(1));
  EXPECT_EQ("12", NumberToString<int>(12));
  EXPECT_EQ("123", NumberToString<int>(123));
  EXPECT_EQ("1234", NumberToString<int>(1234));
  EXPECT_EQ("12345", NumberToString<int>(12345));
  EXPECT_EQ("123456", NumberToString<int>(123456));
  EXPECT_EQ("1234567", NumberToString<int>(1234567));
  EXPECT_EQ("12345678", NumberToString<int>(12345678));
  EXPECT_EQ("123456789", NumberToString<int>(123456789));
  EXPECT_EQ("-1", NumberToString<int>(-1));
  EXPECT_EQ("-12", NumberToString<int>(-12));
  EXPECT_EQ("-123", NumberToString<int>(-123));
  EXPECT_EQ("-1234", NumberToString<int>(-1234));
  EXPECT_EQ("-12345", NumberToString<int>(-12345));
  EXPECT_EQ("-123456", NumberToString<int>(-123456));
  EXPECT_EQ("-1234567", NumberToString<int>(-1234567));
  EXPECT_EQ("-12345678", NumberToString<int>(-12345678));
  EXPECT_EQ("-123456789", NumberToString<int>(-123456789));
}

TEST(StringNumberConversionsTest, NumberToString_Basic_Base16) {
  EXPECT_EQ("0", NumberToString<int32_t>(static_cast<int32_t>(0), Base::k16));
  EXPECT_EQ("7B", NumberToString<int32_t>(static_cast<int32_t>(123), Base::k16));
  EXPECT_EQ("-1C8", NumberToString<int32_t>(static_cast<int32_t>(-456), Base::k16));

  EXPECT_EQ("0", NumberToString<uint32_t>(static_cast<uint32_t>(0)));
  EXPECT_EQ("123", NumberToString<uint32_t>(static_cast<uint32_t>(123)));

  EXPECT_EQ("0", NumberToString<int>(0, Base::k16));
  EXPECT_EQ("7B", NumberToString<int>(123, Base::k16));
  EXPECT_EQ("-1C8", NumberToString<int>(-456, Base::k16));

  EXPECT_EQ("0", NumberToString<unsigned>(0u, Base::k16));
  EXPECT_EQ("7B", NumberToString<unsigned>(123u, Base::k16));

  EXPECT_EQ("1", NumberToString<int64_t>(1ll, Base::k16));
  EXPECT_EQ("12", NumberToString<int64_t>(0x12ll, Base::k16));
  EXPECT_EQ("123", NumberToString<int64_t>(0x123ll, Base::k16));
  EXPECT_EQ("12345", NumberToString<int64_t>(0x12345ll, Base::k16));
  EXPECT_EQ("123456", NumberToString<int64_t>(0x123456ll, Base::k16));
  EXPECT_EQ("1234567", NumberToString<int64_t>(0x1234567ll, Base::k16));
  EXPECT_EQ("12345678", NumberToString<int64_t>(0x12345678ll, Base::k16));
  EXPECT_EQ("23456789", NumberToString<int64_t>(0x23456789ll, Base::k16));
  EXPECT_EQ("3456789A", NumberToString<int64_t>(0x3456789All, Base::k16));
  EXPECT_EQ("456789AB", NumberToString<int64_t>(0x456789ABll, Base::k16));
  EXPECT_EQ("56789ABC", NumberToString<int64_t>(0x56789ABCll, Base::k16));
  EXPECT_EQ("6789ABCD", NumberToString<int64_t>(0x6789ABCDll, Base::k16));
  EXPECT_EQ("789ABCDE", NumberToString<int64_t>(0x789ABCDEll, Base::k16));
  EXPECT_EQ("89ABCDEF", NumberToString<int64_t>(0x89ABCDEFll, Base::k16));
  EXPECT_EQ("-1", NumberToString<int64_t>(-1ll, Base::k16));
  EXPECT_EQ("-12", NumberToString<int64_t>(-0x12ll, Base::k16));
  EXPECT_EQ("-123", NumberToString<int64_t>(-0x123ll, Base::k16));
  EXPECT_EQ("-12345", NumberToString<int64_t>(-0x12345ll, Base::k16));
  EXPECT_EQ("-123456", NumberToString<int64_t>(-0x123456ll, Base::k16));
  EXPECT_EQ("-1234567", NumberToString<int64_t>(-0x1234567ll, Base::k16));
  EXPECT_EQ("-12345678", NumberToString<int64_t>(-0x12345678ll, Base::k16));
  EXPECT_EQ("-23456789", NumberToString<int64_t>(-0x23456789ll, Base::k16));
  EXPECT_EQ("-3456789A", NumberToString<int64_t>(-0x3456789All, Base::k16));
  EXPECT_EQ("-456789AB", NumberToString<int64_t>(-0x456789ABll, Base::k16));
  EXPECT_EQ("-56789ABC", NumberToString<int64_t>(-0x56789ABCll, Base::k16));
  EXPECT_EQ("-6789ABCD", NumberToString<int64_t>(-0x6789ABCDll, Base::k16));
  EXPECT_EQ("-789ABCDE", NumberToString<int64_t>(-0x789ABCDEll, Base::k16));
  EXPECT_EQ("-89ABCDEF", NumberToString<int64_t>(-0x89ABCDEFll, Base::k16));
}

TEST(StringNumberConversionsTest, NumberToString_StdintTypes) {
  // int8_t
  EXPECT_EQ("0", NumberToString<int8_t>(static_cast<int8_t>(0)));
  EXPECT_EQ("127", NumberToString<int8_t>(std::numeric_limits<int8_t>::max()));
  EXPECT_EQ("-128", NumberToString<int8_t>(std::numeric_limits<int8_t>::min()));
  EXPECT_EQ("0", NumberToString<int8_t>(static_cast<int8_t>(0), Base::k16));
  EXPECT_EQ("7F", NumberToString<int8_t>(std::numeric_limits<int8_t>::max(), Base::k16));
  EXPECT_EQ("-80", NumberToString<int8_t>(std::numeric_limits<int8_t>::min(), Base::k16));

  // uint8_t
  EXPECT_EQ("0", NumberToString<uint8_t>(static_cast<uint8_t>(0)));
  EXPECT_EQ("255", NumberToString<uint8_t>(std::numeric_limits<uint8_t>::max()));
  EXPECT_EQ("0", NumberToString<uint8_t>(static_cast<uint8_t>(0), Base::k16));
  EXPECT_EQ("FF", NumberToString<uint8_t>(std::numeric_limits<uint8_t>::max(), Base::k16));

  // int16_t
  EXPECT_EQ("0", NumberToString<int16_t>(static_cast<int16_t>(0)));
  EXPECT_EQ("32767", NumberToString<int16_t>(std::numeric_limits<int16_t>::max()));
  EXPECT_EQ("-32768", NumberToString<int16_t>(std::numeric_limits<int16_t>::min()));
  EXPECT_EQ("0", NumberToString<int16_t>(static_cast<int16_t>(0), Base::k16));
  EXPECT_EQ("7FFF", NumberToString<int16_t>(std::numeric_limits<int16_t>::max(), Base::k16));
  EXPECT_EQ("-8000", NumberToString<int16_t>(std::numeric_limits<int16_t>::min(), Base::k16));

  // uint16_t
  EXPECT_EQ("0", NumberToString<uint16_t>(static_cast<uint16_t>(0)));
  EXPECT_EQ("65535", NumberToString<uint16_t>(std::numeric_limits<uint16_t>::max()));
  EXPECT_EQ("0", NumberToString<uint16_t>(static_cast<uint16_t>(0), Base::k16));
  EXPECT_EQ("FFFF", NumberToString<uint16_t>(std::numeric_limits<uint16_t>::max(), Base::k16));

  // int32_t
  EXPECT_EQ("0", NumberToString<int32_t>(static_cast<int32_t>(0)));
  EXPECT_EQ("2147483647", NumberToString<int32_t>(std::numeric_limits<int32_t>::max()));
  EXPECT_EQ("-2147483648", NumberToString<int32_t>(std::numeric_limits<int32_t>::min()));
  EXPECT_EQ("0", NumberToString<int32_t>(static_cast<int32_t>(0), Base::k16));
  EXPECT_EQ("7FFFFFFF", NumberToString<int32_t>(std::numeric_limits<int32_t>::max(), Base::k16));
  EXPECT_EQ("-80000000", NumberToString<int32_t>(std::numeric_limits<int32_t>::min(), Base::k16));

  // uint32_t
  EXPECT_EQ("0", NumberToString<uint32_t>(static_cast<uint32_t>(0)));
  EXPECT_EQ("4294967295", NumberToString<uint32_t>(std::numeric_limits<uint32_t>::max()));
  EXPECT_EQ("0", NumberToString<uint32_t>(static_cast<uint32_t>(0), Base::k16));
  EXPECT_EQ("FFFFFFFF", NumberToString<uint32_t>(std::numeric_limits<uint32_t>::max(), Base::k16));

  // int64_t
  EXPECT_EQ("0", NumberToString<int64_t>(static_cast<int64_t>(0)));
  EXPECT_EQ("9223372036854775807", NumberToString<int64_t>(std::numeric_limits<int64_t>::max()));
  EXPECT_EQ("-9223372036854775808", NumberToString<int64_t>(std::numeric_limits<int64_t>::min()));
  EXPECT_EQ("0", NumberToString<int64_t>(static_cast<int64_t>(0), Base::k16));
  EXPECT_EQ("7FFFFFFFFFFFFFFF",
            NumberToString<int64_t>(std::numeric_limits<int64_t>::max(), Base::k16));
  EXPECT_EQ("-8000000000000000",
            NumberToString<int64_t>(std::numeric_limits<int64_t>::min(), Base::k16));

  // uint64_t
  EXPECT_EQ("0", NumberToString<uint64_t>(static_cast<uint64_t>(0)));
  EXPECT_EQ("18446744073709551615", NumberToString<uint64_t>(std::numeric_limits<uint64_t>::max()));
  EXPECT_EQ("0", NumberToString<uint64_t>(static_cast<uint64_t>(0), Base::k16));
  EXPECT_EQ("FFFFFFFFFFFFFFFF",
            NumberToString<uint64_t>(std::numeric_limits<uint64_t>::max(), Base::k16));
}

TEST(StringNumberConversionsTest, StringToNumberWithError_Basic) {
  {
    int32_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<int32_t>("0", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("123", &number));
    EXPECT_EQ(123, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("-456", &number));
    EXPECT_EQ(-456, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("123", &number, Base::k16));
    EXPECT_EQ(291, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("A", &number, Base::k16));
    EXPECT_EQ(10, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("abCDeF", &number, Base::k16));
    EXPECT_EQ(11259375, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("-abCDeF", &number, Base::k16));
    EXPECT_EQ(-11259375, number);
  }

  {
    uint32_t number = 42u;
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("0", &number));
    EXPECT_EQ(0u, number);
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("123", &number));
    EXPECT_EQ(123u, number);
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("abCDeF", &number, Base::k16));
    EXPECT_EQ(11259375u, number);
  }

  {
    int number = 42;
    EXPECT_TRUE(StringToNumberWithError<int>("0", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int>("123", &number));
    EXPECT_EQ(123, number);
    EXPECT_TRUE(StringToNumberWithError<int>("-456", &number));
    EXPECT_EQ(-456, number);
    EXPECT_TRUE(StringToNumberWithError<int>("-abCDeF", &number, Base::k16));
    EXPECT_EQ(-11259375, number);
  }

  {
    unsigned number = 42u;
    EXPECT_TRUE(StringToNumberWithError<unsigned>("0", &number));
    EXPECT_EQ(0u, number);
    EXPECT_TRUE(StringToNumberWithError<unsigned>("123", &number));
    EXPECT_EQ(123u, number);
    EXPECT_TRUE(StringToNumberWithError<unsigned>("abCDeF", &number, Base::k16));
    EXPECT_EQ(11259375u, number);
  }
}

TEST(StringNumberConversionsTest, StringToNumberWithError_Errors) {
  {
    int32_t number = 42;
    EXPECT_FALSE(StringToNumberWithError<int32_t>("", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("/", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>(":", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("A", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("0x", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("123x", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("+123", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("@", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("G", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("`", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("g", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("999999999999999", &number));
    EXPECT_EQ(42, number);
  }

  {
    uint32_t number = 42u;
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("/", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>(":", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("A", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("0x", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("123x", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("+123", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("999999999999999", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("-123", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("-0", &number));
    EXPECT_EQ(42u, number);
  }
}

TEST(StringNumberConversionsTest, StringToNumberWithError_LeadingZeros) {
  {
    int32_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<int32_t>("00", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("0123", &number));
    EXPECT_EQ(123, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("-0", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("-00", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("-0456", &number));
    EXPECT_EQ(-456, number);
  }

  {
    uint32_t number = 42u;
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("00", &number));
    EXPECT_EQ(0u, number);
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("0123", &number));
    EXPECT_EQ(123u, number);
  }
}

TEST(StringNumberConversionsTest, StringToNumberWithError_StdintTypes) {
  {
    int8_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<int8_t>("0", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int8_t>("0", &number, Base::k16));
    EXPECT_EQ(0, number);

    EXPECT_TRUE(StringToNumberWithError<int8_t>("127", &number));
    EXPECT_EQ(std::numeric_limits<int8_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<int8_t>("7f", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int8_t>::max(), number);

    EXPECT_TRUE(StringToNumberWithError<int8_t>("-128", &number));
    EXPECT_EQ(std::numeric_limits<int8_t>::min(), number);
    EXPECT_TRUE(StringToNumberWithError<int8_t>("-80", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int8_t>::min(), number);

    EXPECT_FALSE(StringToNumberWithError<int8_t>("128", &number));
    EXPECT_FALSE(StringToNumberWithError<int8_t>("80", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int8_t>("-129", &number));
    EXPECT_FALSE(StringToNumberWithError<int8_t>("-81", &number, Base::k16));
  }

  {
    uint8_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<uint8_t>("0", &number));
    EXPECT_EQ(0u, number);
    EXPECT_TRUE(StringToNumberWithError<uint8_t>("0", &number, Base::k16));
    EXPECT_EQ(0u, number);

    EXPECT_TRUE(StringToNumberWithError<uint8_t>("255", &number));
    EXPECT_EQ(std::numeric_limits<uint8_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<uint8_t>("ff", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<uint8_t>::max(), number);

    EXPECT_FALSE(StringToNumberWithError<uint8_t>("256", &number));
    EXPECT_FALSE(StringToNumberWithError<uint8_t>("100", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<uint8_t>("-1", &number));
    EXPECT_FALSE(StringToNumberWithError<uint8_t>("-1", &number, Base::k16));
  }

  {
    int16_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<int16_t>("0", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int16_t>("0", &number, Base::k16));
    EXPECT_EQ(0, number);

    EXPECT_TRUE(StringToNumberWithError<int16_t>("32767", &number));
    EXPECT_EQ(std::numeric_limits<int16_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<int16_t>("7fff", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int16_t>::max(), number);

    EXPECT_TRUE(StringToNumberWithError<int16_t>("-32768", &number));
    EXPECT_EQ(std::numeric_limits<int16_t>::min(), number);
    EXPECT_TRUE(StringToNumberWithError<int16_t>("-8000", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int16_t>::min(), number);

    EXPECT_FALSE(StringToNumberWithError<int16_t>("32768", &number));
    EXPECT_FALSE(StringToNumberWithError<int16_t>("8000", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int16_t>("-32769", &number));
    EXPECT_FALSE(StringToNumberWithError<int16_t>("-80001", &number, Base::k16));
  }

  {
    uint16_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<uint16_t>("0", &number));
    EXPECT_EQ(0u, number);
    EXPECT_TRUE(StringToNumberWithError<uint16_t>("0", &number, Base::k16));
    EXPECT_EQ(0u, number);

    EXPECT_TRUE(StringToNumberWithError<uint16_t>("65535", &number));
    EXPECT_EQ(std::numeric_limits<uint16_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<uint16_t>("ffff", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<uint16_t>::max(), number);

    EXPECT_FALSE(StringToNumberWithError<uint16_t>("65536", &number));
    EXPECT_FALSE(StringToNumberWithError<uint16_t>("10000", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<uint16_t>("-1", &number));
    EXPECT_FALSE(StringToNumberWithError<uint16_t>("-1", &number, Base::k16));
  }

  {
    int32_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<int32_t>("0", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("0", &number, Base::k16));
    EXPECT_EQ(0, number);

    EXPECT_TRUE(StringToNumberWithError<int32_t>("2147483647", &number));
    EXPECT_EQ(std::numeric_limits<int32_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("7FFFFFFF", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int32_t>::max(), number);

    EXPECT_TRUE(StringToNumberWithError<int32_t>("-2147483648", &number));
    EXPECT_EQ(std::numeric_limits<int32_t>::min(), number);
    EXPECT_TRUE(StringToNumberWithError<int32_t>("-80000000", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int32_t>::min(), number);

    EXPECT_FALSE(StringToNumberWithError<int32_t>("2147483648", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("80000000", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("-2147483649", &number));
    EXPECT_FALSE(StringToNumberWithError<int32_t>("-80000001", &number, Base::k16));
  }

  {
    uint32_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("0", &number));
    EXPECT_EQ(0u, number);
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("0", &number, Base::k16));
    EXPECT_EQ(0u, number);

    EXPECT_TRUE(StringToNumberWithError<uint32_t>("4294967295", &number));
    EXPECT_EQ(std::numeric_limits<uint32_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<uint32_t>("FFFFFFFF", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<uint32_t>::max(), number);

    EXPECT_FALSE(StringToNumberWithError<uint32_t>("4294967296", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("100000000", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("-1", &number));
    EXPECT_FALSE(StringToNumberWithError<uint32_t>("-1", &number, Base::k16));
  }

  {
    int64_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<int64_t>("0", &number));
    EXPECT_EQ(0, number);
    EXPECT_TRUE(StringToNumberWithError<int64_t>("0", &number, Base::k16));
    EXPECT_EQ(0, number);

    EXPECT_TRUE(StringToNumberWithError<int64_t>("9223372036854775807", &number));
    EXPECT_EQ(std::numeric_limits<int64_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<int64_t>("7FFFFFFFFFFFFFFF", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int64_t>::max(), number);

    EXPECT_TRUE(StringToNumberWithError<int64_t>("-9223372036854775808", &number));
    EXPECT_EQ(std::numeric_limits<int64_t>::min(), number);
    EXPECT_TRUE(StringToNumberWithError<int64_t>("-8000000000000000", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<int64_t>::min(), number);

    EXPECT_FALSE(StringToNumberWithError<int64_t>("9223372036854775808", &number));
    EXPECT_FALSE(StringToNumberWithError<int64_t>("8000000000000000", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<int64_t>("-9223372036854775809", &number));
    EXPECT_FALSE(StringToNumberWithError<int64_t>("-8000000000000001", &number, Base::k16));
  }

  {
    uint64_t number = 42;
    EXPECT_TRUE(StringToNumberWithError<uint64_t>("0", &number));
    EXPECT_EQ(0u, number);
    EXPECT_TRUE(StringToNumberWithError<uint64_t>("0", &number, Base::k16));
    EXPECT_EQ(0u, number);

    EXPECT_TRUE(StringToNumberWithError<uint64_t>("18446744073709551615", &number));
    EXPECT_EQ(std::numeric_limits<uint64_t>::max(), number);
    EXPECT_TRUE(StringToNumberWithError<uint64_t>("FFFFFFFFFFFFFFFF", &number, Base::k16));
    EXPECT_EQ(std::numeric_limits<uint64_t>::max(), number);

    EXPECT_FALSE(StringToNumberWithError<uint64_t>("18446744073709551616", &number));
    EXPECT_FALSE(StringToNumberWithError<uint64_t>("80000000000000000", &number, Base::k16));
    EXPECT_FALSE(StringToNumberWithError<uint64_t>("-1", &number));
    EXPECT_FALSE(StringToNumberWithError<uint64_t>("-1", &number, Base::k16));
  }
}

TEST(StringNumberConversionsTest, StringToNumber_Basic) {
  EXPECT_EQ(0, StringToNumber<int32_t>("0"));
  EXPECT_EQ(123, StringToNumber<int32_t>("123"));
  EXPECT_EQ(-456, StringToNumber<int32_t>("-456"));
  EXPECT_EQ(123, StringToNumber<int32_t>("7B", Base::k16));
  EXPECT_EQ(-123, StringToNumber<int32_t>("-7b", Base::k16));

  EXPECT_EQ(0u, StringToNumber<uint32_t>("0"));
  EXPECT_EQ(123u, StringToNumber<uint32_t>("123"));
  EXPECT_EQ(123u, StringToNumber<uint32_t>("7b", Base::k16));

  EXPECT_EQ(0, StringToNumber<int>("0"));
  EXPECT_EQ(123, StringToNumber<int>("123"));
  EXPECT_EQ(-456, StringToNumber<int>("-456"));
  EXPECT_EQ(123, StringToNumber<int>("7B", Base::k16));
  EXPECT_EQ(-123, StringToNumber<int>("-7b", Base::k16));

  EXPECT_EQ(0u, StringToNumber<unsigned>("0"));
  EXPECT_EQ(123u, StringToNumber<unsigned>("123"));
  EXPECT_EQ(123u, StringToNumber<unsigned>("7b", Base::k16));
}

TEST(StringNumberConversionsTest, StringToNumber_Errors) {
  EXPECT_EQ(0, StringToNumber<int32_t>(""));
  EXPECT_EQ(0, StringToNumber<int32_t>("/"));
  EXPECT_EQ(0, StringToNumber<int32_t>(":"));
  EXPECT_EQ(0, StringToNumber<int32_t>("A"));
  EXPECT_EQ(0, StringToNumber<int32_t>("0x"));
  EXPECT_EQ(0, StringToNumber<int32_t>("123x"));
  EXPECT_EQ(0, StringToNumber<int32_t>("+123"));
  EXPECT_EQ(0, StringToNumber<int32_t>("999999999999999"));
  EXPECT_EQ(0, StringToNumber<int32_t>("", Base::k16));
  EXPECT_EQ(0, StringToNumber<int32_t>("/", Base::k16));
  EXPECT_EQ(0, StringToNumber<int32_t>(":", Base::k16));
  EXPECT_EQ(0, StringToNumber<int32_t>("G", Base::k16));
  EXPECT_EQ(0, StringToNumber<int32_t>("0x", Base::k16));
  EXPECT_EQ(0, StringToNumber<int32_t>("7fx", Base::k16));
  EXPECT_EQ(0, StringToNumber<int32_t>("+7B", Base::k16));

  EXPECT_EQ(0u, StringToNumber<uint32_t>(""));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("/"));
  EXPECT_EQ(0u, StringToNumber<uint32_t>(":"));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("A"));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("0x"));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("123x"));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("+123"));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("999999999999999"));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("", Base::k16));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("/", Base::k16));
  EXPECT_EQ(0u, StringToNumber<uint32_t>(":", Base::k16));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("G", Base::k16));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("0x", Base::k16));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("7fx", Base::k16));
  EXPECT_EQ(0u, StringToNumber<uint32_t>("+7B", Base::k16));
}

}  // namespace
}  // namespace fxl
