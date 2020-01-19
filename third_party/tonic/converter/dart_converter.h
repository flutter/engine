// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_CONVERTER_TONIC_DART_CONVERTER_H_
#define LIB_CONVERTER_TONIC_DART_CONVERTER_H_

#include <string>
#include <type_traits>
#include <vector>

#include "third_party/dart/runtime/include/dart_api.h"
#include "tonic/common/macros.h"

namespace tonic {

// DartConvert converts types back and forth from Sky to Dart. The template
// parameter |T| determines what kind of type conversion to perform.
template <typename T, typename Enable = void>
struct DartConverter {};

// This is to work around the fact that typedefs do not create new types. If you
// have a typedef, and want it to use a different converter, specialize this
// template and override the types here.
// Ex:
//   typedef int ColorType;  // Want to use a different converter.
//   class ColorConverterType {};  // Dummy type.
//   template<> struct DartConvertType<ColorConverterType> {
//     using ConverterType = ColorConverterType;
//     using ValueType = ColorType;
//   };
template <typename T>
struct DartConverterTypes {
  using ConverterType = T;
  using ValueType = T;
};

////////////////////////////////////////////////////////////////////////////////
// Boolean

template <>
struct DartConverter<bool> {
  static Dart_Handle ToDart(bool val) { return Dart_NewBoolean(val); }

  static void SetReturnValue(Dart_NativeArguments args, bool val) {
    Dart_SetBooleanReturnValue(args, val);
  }

  static bool FromDart(Dart_Handle handle) {
    bool result = 0;
    Dart_BooleanValue(handle, &result);
    return result;
  }

  static bool FromArguments(Dart_NativeArguments args,
                            int index,
                            Dart_Handle& exception) {
    bool result = false;
    Dart_GetNativeBooleanArgument(args, index, &result);
    return result;
  }
};

////////////////////////////////////////////////////////////////////////////////
// Numbers

// Unsigned integers
// TODO(dworsham): Dart doesn't support unsigned integers as return values or
// parameters, so we don't expose that here.
template <typename T>
struct DartConverter<
    T,
    typename std::enable_if<std::is_integral<T>::value &&
                            std::is_unsigned<T>::value>::type> {
  static Dart_Handle ToDart(T val) { return Dart_NewIntegerFromUint64(val); }

  static void SetReturnValue(Dart_NativeArguments args, T val) {
    int64_t new_val = static_cast<int64_t>(val);
    TONIC_CHECK(new_val >= 0);
    Dart_SetIntegerReturnValue(args, new_val);
  }

  static T FromDart(Dart_Handle handle) {
    uint64_t result = 0;
    Dart_IntegerToUint64(handle, &result);
    return static_cast<T>(result);
  }

  static T FromArguments(Dart_NativeArguments args,
                         int index,
                         Dart_Handle& exception) {
    int64_t result = 0;
    Dart_GetNativeIntegerArgument(args, index, &result);
    TONIC_CHECK(result >= 0);

    return static_cast<T>(result);
  }
};

// Signed integers
template <typename T>
struct DartConverter<
    T,
    typename std::enable_if<std::is_integral<T>::value &&
                            !std::is_unsigned<T>::value>::type> {
  static Dart_Handle ToDart(T val) { return Dart_NewInteger(val); }

  static void SetReturnValue(Dart_NativeArguments args, T val) {
    Dart_SetIntegerReturnValue(args, val);
  }

  static T FromDart(Dart_Handle handle) {
    int64_t result = 0;
    Dart_IntegerToInt64(handle, &result);
    return static_cast<T>(result);
  }

  static T FromArguments(Dart_NativeArguments args,
                         int index,
                         Dart_Handle& exception) {
    int64_t result = 0;
    Dart_GetNativeIntegerArgument(args, index, &result);
    return static_cast<T>(result);
  }
};

// Floating point
template <typename T>
struct DartConverter<
    T,
    typename std::enable_if<std::is_floating_point<T>::value>::type> {
  static Dart_Handle ToDart(T val) { return Dart_NewDouble(val); }

  static void SetReturnValue(Dart_NativeArguments args, T val) {
    Dart_SetDoubleReturnValue(args, val);
  }

  static T FromDart(Dart_Handle handle) {
    double result = 0;
    Dart_DoubleValue(handle, &result);
    return static_cast<T>(result);
  }

  static T FromArguments(Dart_NativeArguments args,
                         int index,
                         Dart_Handle& exception) {
    double result = 0;
    Dart_GetNativeDoubleArgument(args, index, &result);
    return static_cast<T>(result);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Enum Classes

template <typename T>
struct DartConverter<T, typename std::enable_if<std::is_enum<T>::value>::type> {
  static Dart_Handle ToDart(T val) {
    return Dart_NewInteger(
        static_cast<typename std::underlying_type<T>::type>(val));
  }

  static void SetReturnValue(Dart_NativeArguments args, T val) {
    Dart_SetIntegerReturnValue(
        args, static_cast<typename std::underlying_type<T>::type>(val));
  }

  static T FromDart(Dart_Handle handle) {
    int64_t result = 0;
    Dart_IntegerToInt64(handle, &result);
    return static_cast<T>(result);
  }

  static T FromArguments(Dart_NativeArguments args,
                         int index,
                         Dart_Handle& exception) {
    int64_t result = 0;
    Dart_GetNativeIntegerArgument(args, index, &result);
    return static_cast<T>(result);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Strings

template <>
struct DartConverter<std::string> {
  static Dart_Handle ToDart(const std::string& val) {
    return Dart_NewStringFromUTF8(reinterpret_cast<const uint8_t*>(val.data()),
                                  val.length());
  }

  static void SetReturnValue(Dart_NativeArguments args,
                             const std::string& val) {
    Dart_SetReturnValue(args, ToDart(val));
  }

  static std::string FromDart(Dart_Handle handle) {
    uint8_t* data = nullptr;
    intptr_t length = 0;
    if (Dart_IsError(Dart_StringToUTF8(handle, &data, &length)))
      return std::string();
    return std::string(reinterpret_cast<char*>(data), length);
  }

  static std::string FromArguments(Dart_NativeArguments args,
                                   int index,
                                   Dart_Handle& exception) {
    return FromDart(Dart_GetNativeArgument(args, index));
  }
};

template <>
struct DartConverter<std::u16string> {
  static Dart_Handle ToDart(const std::u16string& val) {
    return Dart_NewStringFromUTF16(
        reinterpret_cast<const uint16_t*>(val.data()), val.length());
  }

  static void SetReturnValue(Dart_NativeArguments args,
                             const std::u16string& val) {
    Dart_SetReturnValue(args, ToDart(val));
  }

  static std::u16string FromDart(Dart_Handle handle) {
    intptr_t length = 0;
    Dart_StringLength(handle, &length);
    std::vector<uint16_t> data(length);
    Dart_StringToUTF16(handle, data.data(), &length);
    return std::u16string(reinterpret_cast<char16_t*>(data.data()), length);
  }

  static std::u16string FromArguments(Dart_NativeArguments args,
                                      int index,
                                      Dart_Handle& exception) {
    return FromDart(Dart_GetNativeArgument(args, index));
  }
};

template <>
struct DartConverter<const char*> {
  static Dart_Handle ToDart(const char* val) {
    return Dart_NewStringFromCString(val);
  }

  static void SetReturnValue(Dart_NativeArguments args, const char* val) {
    Dart_SetReturnValue(args, ToDart(val));
  }

  static const char* FromDart(Dart_Handle handle) {
    const char* result = nullptr;
    Dart_StringToCString(handle, &result);
    return result;
  }

  static const char* FromArguments(Dart_NativeArguments args,
                                   int index,
                                   Dart_Handle& exception) {
    return FromDart(Dart_GetNativeArgument(args, index));
  }
};

////////////////////////////////////////////////////////////////////////////////
// Collections

template <typename T, typename Enable = void>
struct DartListFactory {
  static Dart_Handle NewList(intptr_t length) { return Dart_NewList(length); }
};

// Integers
// TODO(dworsham): Dart doesn't support unsigned integers as a CoreType here so
// we don't expose them.
template <typename T>
struct DartListFactory<
    T,
    typename std::enable_if<std::is_integral<T>::value &&
                            !std::is_unsigned<T>::value>::type> {
  static Dart_Handle NewList(size_t length) {
    return Dart_NewListOf(Dart_CoreType_Int, length);
  }
};

template <>
struct DartListFactory<std::string> {
  static Dart_Handle NewList(size_t length) {
    return Dart_NewListOf(Dart_CoreType_String, length);
  }
};

template <typename T>
struct DartConverter<std::vector<T>> {
  using ValueType = typename DartConverterTypes<T>::ValueType;
  using ConverterType = typename DartConverterTypes<T>::ConverterType;

  static Dart_Handle ToDart(const std::vector<ValueType>& val) {
    Dart_Handle list = DartListFactory<ValueType>::NewList(val.size());
    if (Dart_IsError(list))
      return list;
    for (size_t i = 0; i < val.size(); i++) {
      Dart_Handle result =
          Dart_ListSetAt(list, i, DartConverter<ConverterType>::ToDart(val[i]));
      if (Dart_IsError(result))
        return result;
    }
    return list;
  }

  static void SetReturnValue(Dart_NativeArguments args,
                             const std::vector<ValueType>& val) {
    Dart_SetReturnValue(args, ToDart(val));
  }

  static std::vector<ValueType> FromDart(Dart_Handle handle) {
    std::vector<ValueType> result;

    if (!Dart_IsList(handle))
      return result;

    intptr_t length = 0;
    Dart_ListLength(handle, &length);

    if (length == 0)
      return result;

    result.reserve(length);

    std::vector<Dart_Handle> items(length);
    Dart_Handle items_result =
        Dart_ListGetRange(handle, 0, length, items.data());
    TONIC_DCHECK(!Dart_IsError(items_result));

    for (intptr_t i = 0; i < length; ++i) {
      TONIC_DCHECK(items[i]);
      result.push_back(DartConverter<ConverterType>::FromDart(items[i]));
    }
    return result;
  }

  static std::vector<ValueType> FromArguments(Dart_NativeArguments args,
                                              int index,
                                              Dart_Handle& exception) {
    return FromDart(Dart_GetNativeArgument(args, index));
  }
};

////////////////////////////////////////////////////////////////////////////////
// Dart_Handle

template <>
struct DartConverter<Dart_Handle> {
  static Dart_Handle ToDart(Dart_Handle val) { return val; }

  static void SetReturnValue(Dart_NativeArguments args, Dart_Handle val) {
    Dart_SetReturnValue(args, val);
  }

  static Dart_Handle FromDart(Dart_Handle handle) { return handle; }

  static Dart_Handle FromArguments(Dart_NativeArguments args,
                                   int index,
                                   Dart_Handle& exception) {
    return Dart_GetNativeArgument(args, index);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Convience wrappers using type inference

template <typename T>
Dart_Handle ToDart(const T& object) {
  return DartConverter<T>::ToDart(object);
}

////////////////////////////////////////////////////////////////////////////////
// std::string support

inline Dart_Handle StdStringToDart(const std::string& val) {
  return DartConverter<std::string>::ToDart(val);
}

inline std::string StdStringFromDart(Dart_Handle handle) {
  return DartConverter<std::string>::FromDart(handle);
}

// Alias Dart_NewStringFromCString for less typing.
inline Dart_Handle ToDart(const char* val) {
  return Dart_NewStringFromCString(val);
}

}  // namespace tonic

#endif  // LIB_CONVERTER_TONIC_DART_CONVERTER_H_
