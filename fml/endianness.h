// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_ENDIANNESS_H_
#define FLUTTER_FML_ENDIANNESS_H_

#include <cstdint>
#include <type_traits>
#if defined(_MSC_VER)
#include "intrin.h"
#endif

#include "flutter/fml/build_config.h"

// Compiler intrinsics for flipping endianness.
#define BYTESWAP16(n) __builtin_bswap16(n)
#define BYTESWAP32(n) __builtin_bswap32(n)
#define BYTESWAP64(n) __builtin_bswap64(n)

#if defined(_MSC_VER)
#define BYTESWAP16(n) _byteswap_ushort(n)
#define BYTESWAP32(n) _byteswap_ulong(n)
#define BYTESWAP64(n) _byteswap_uint64(n)
#endif

namespace fml {

/// @brief  Flips the endianness of the given value.
template <typename T>
constexpr T ByteSwap(T n) {
  if constexpr (sizeof(T) == 1) {
    return n;
  } else if constexpr (sizeof(T) == 2) {
    return (T)BYTESWAP16((uint16_t)n);
  } else if constexpr (sizeof(T) == 4) {
    return (T)BYTESWAP32((uint32_t)n);
  } else if constexpr (sizeof(T) == 8) {
    return (T)BYTESWAP64((uint64_t)n);
  } else {
    static_assert(!sizeof(T), "Unsupported size");
  }
}

/// @brief  Convert a known big endian value to match the endianness of the
///         current architecture. This is effectively a cross platform
///         ntohl/ntohs (as network byte order is always Big Endian).
template <typename T>
constexpr T BigEndianToArch(T n) {
#if ARCH_CPU_LITTLE_ENDIAN
  return ByteSwap<T>(n);
#else
  return n;
#endif
}

/// @brief  Convert a known little endian value to match the endianness of the
///         current architecture.
template <typename T>
constexpr T LittleEndianToArch(T n) {
#if !ARCH_CPU_LITTLE_ENDIAN
  return ByteSwap<T>(n);
#else
  return n;
#endif
}

}  // namespace fml

#endif  // FLUTTER_FML_ENDIANNESS_H_
