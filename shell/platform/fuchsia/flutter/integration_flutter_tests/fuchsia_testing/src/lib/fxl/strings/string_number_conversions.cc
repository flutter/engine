// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/strings/string_number_conversions.h"

#include <assert.h>
#include <stdint.h>

#include <limits>
#include <type_traits>

namespace fxl {
namespace {

template <typename NumberType>
bool GetDigitValue(const char s, Base base, NumberType* out_digit) {
  assert(out_digit);

  if (s < '0')
    return false;

  if (s <= '9') {
    *out_digit = static_cast<NumberType>(s - '0');
    return true;
  }

  if (base != Base::k16)
    return false;

  if (s >= 'a' && s <= 'f') {
    *out_digit = static_cast<NumberType>(s - 'a' + 10);
    return true;
  }

  if (s >= 'A' && s <= 'F') {
    *out_digit = static_cast<NumberType>(s - 'A' + 10);
    return true;
  }

  return false;
}

// Helper for |StringToNumberWithError()|. Note that this may modify |*number|
// even on failure.
template <typename NumberType>
bool StringToPositiveNumberWithError(const char* s, size_t length, Base base, NumberType* number) {
  const NumberType kBase = static_cast<NumberType>(base == Base::k10 ? 10 : 16);
  constexpr NumberType kMaxAllowed = std::numeric_limits<NumberType>::max();

  assert(s);
  assert(length > 0u);
  assert(number);

  *number = 0;
  for (size_t i = 0; i < length; i++) {
    NumberType new_digit;
    if (!GetDigitValue(s[i], base, &new_digit))
      return false;

    // This is really a check of "*number * kBase + new_digit > kMaxAllowed":
    if (*number > kMaxAllowed / kBase ||
        (*number == kMaxAllowed / kBase && new_digit > kMaxAllowed % kBase))
      return false;
    *number = *number * kBase + new_digit;
  }

  return true;
}

// Helper for |StringToNumberWithError()|. Note that this may modify |*number|
// even on failure.
template <typename NumberType>
bool StringToNegativeNumberWithError(const char* s, size_t length, Base base, NumberType* number) {
  const NumberType kBase = static_cast<NumberType>(base == Base::k10 ? 10 : 16);
  constexpr NumberType kMinAllowed = std::numeric_limits<NumberType>::min();

  assert(s);
  assert(length > 0u);
  assert(number);

  *number = 0;
  for (size_t i = 0; i < length; i++) {
    NumberType new_digit;
    if (!GetDigitValue(s[i], base, &new_digit))
      return false;

    // This is really a check of "*number * kBase - new_digit > kMinAllowed":
    if (*number < kMinAllowed / kBase ||
        (kMinAllowed / kBase == *number && new_digit > -(kMinAllowed % kBase)))
      return false;
    *number = *number * kBase - new_digit;
  }

  return true;
}

}  // namespace

template <typename NumberType>
std::string NumberToString(NumberType number, Base base) {
  // Special-case zero (since nonzero cases naturally produce digits).
  if (!number)
    return std::string("0");

  using UnsignedNumberType = typename std::make_unsigned<NumberType>::type;
  // Note: The negative case is safe, since the standard requires that, e.g.,
  // for n a negative int32_t, |static_cast<uint32_t>(n)| = 2^32 - n and for a
  // uint32_t m, |-m| = 2^32 - m.
  bool number_is_negative = (number < static_cast<NumberType>(0));
  UnsignedNumberType abs_number = number_is_negative ? -static_cast<UnsignedNumberType>(number)
                                                     : static_cast<UnsignedNumberType>(number);

  char buf[50];  // Big enough to hold the result from even a 128-bit number.
  size_t i = sizeof(buf);
  while (abs_number) {
    i--;
    if (base == Base::k10) {
      buf[i] = '0' + abs_number % 10u;
      abs_number /= 10u;
    } else {
      UnsignedNumberType val = abs_number % 16u;
      buf[i] = (val < 10) ? static_cast<char>('0' + val) : ('A' + val % 10u);
      abs_number /= 16u;
    }
  }
  if (number_is_negative) {
    i--;
    buf[i] = '-';
  }

  return std::string(buf + i, buf + sizeof(buf));
}

template <typename NumberType>
bool StringToNumberWithError(std::string_view string, NumberType* number, Base base) {
  assert(number);

  if (string.empty())
    return false;

  const char* s = string.data();
  size_t length = string.size();
  NumberType result = 0;
  if (std::is_signed<NumberType>::value && string[0] == '-') {
    if (length < 2)
      return false;
    if (!StringToNegativeNumberWithError<NumberType>(s + 1, length - 1u, base, &result))
      return false;
  } else {
    if (!StringToPositiveNumberWithError<NumberType>(s, length, base, &result))
      return false;
  }

  *number = result;
  return true;
}

// Explicit instantiations for (u)intN_t; count on (unsigned) int being one
// of these:
template std::string NumberToString<int8_t>(int8_t number, Base base);
template std::string NumberToString<uint8_t>(uint8_t number, Base base);
template std::string NumberToString<int16_t>(int16_t number, Base base);
template std::string NumberToString<uint16_t>(uint16_t number, Base base);
template std::string NumberToString<int32_t>(int32_t number, Base base);
template std::string NumberToString<uint32_t>(uint32_t number, Base base);
template std::string NumberToString<int64_t>(int64_t number, Base base);
template std::string NumberToString<uint64_t>(uint64_t number, Base base);
template bool StringToNumberWithError<int8_t>(std::string_view string, int8_t* number, Base base);
template bool StringToNumberWithError<uint8_t>(std::string_view string, uint8_t* number, Base base);
template bool StringToNumberWithError<int16_t>(std::string_view string, int16_t* number, Base base);
template bool StringToNumberWithError<uint16_t>(std::string_view string, uint16_t* number,
                                                Base base);
template bool StringToNumberWithError<int32_t>(std::string_view string, int32_t* number, Base base);
template bool StringToNumberWithError<uint32_t>(std::string_view string, uint32_t* number,
                                                Base base);
template bool StringToNumberWithError<int64_t>(std::string_view string, int64_t* number, Base base);
template bool StringToNumberWithError<uint64_t>(std::string_view string, uint64_t* number,
                                                Base base);

}  // namespace fxl
