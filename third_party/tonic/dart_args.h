// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_TONIC_DART_ARGS_H_
#define LIB_TONIC_DART_ARGS_H_

#include <iostream>
#include <sstream>
#include <type_traits>
#include <utility>

#include "third_party/dart/runtime/include/dart_api.h"
#include "tonic/converter/dart_converter.h"
#include "tonic/dart_wrappable.h"

namespace tonic {

class DartArgIterator {
 public:
  DartArgIterator(Dart_NativeArguments args, int start_index = 1)
      : args_(args), index_(start_index), had_exception_(false) {}

  template <typename T>
  T GetNext() {
    if (had_exception_)
      return T();
    Dart_Handle exception = nullptr;
    T arg = DartConverter<T>::FromArguments(args_, index_++, exception);
    if (exception) {
      had_exception_ = true;
      Dart_ThrowException(exception);
    }
    return arg;
  }

  bool had_exception() const { return had_exception_; }

  Dart_NativeArguments args() const { return args_; }

 private:
  Dart_NativeArguments args_;
  int index_;
  bool had_exception_;

  TONIC_DISALLOW_COPY_AND_ASSIGN(DartArgIterator);
};

// Classes for generating and storing an argument pack of integer indices
// (based on well-known "indices trick", see: http://goo.gl/bKKojn):
template <size_t... indices>
struct IndicesHolder {};

template <size_t requested_index, size_t... indices>
struct IndicesGenerator {
  using type = typename IndicesGenerator<requested_index - 1,
                                         requested_index - 1,
                                         indices...>::type;
};

template <size_t... indices>
struct IndicesGenerator<0, indices...> {
  using type = IndicesHolder<indices...>;
};

template <typename T>
class IndicesForSignature {};

template <typename ResultType, typename... ArgTypes>
struct IndicesForSignature<ResultType (*)(ArgTypes...)> {
  static const size_t count = sizeof...(ArgTypes);
  using type = typename IndicesGenerator<count>::type;
};

template <typename C, typename ResultType, typename... ArgTypes>
struct IndicesForSignature<ResultType (C::*)(ArgTypes...)> {
  static const size_t count = sizeof...(ArgTypes);
  using type = typename IndicesGenerator<count>::type;
};

template <typename C, typename ResultType, typename... ArgTypes>
struct IndicesForSignature<ResultType (C::*)(ArgTypes...) const> {
  static const size_t count = sizeof...(ArgTypes);
  using type = typename IndicesGenerator<count>::type;
};

template <size_t index, typename ArgType>
struct DartArgHolder {
  using ValueType = typename std::remove_const<
      typename std::remove_reference<ArgType>::type>::type;

  ValueType value;

  explicit DartArgHolder(DartArgIterator* it)
      : value(it->GetNext<ValueType>()) {}
};

template <typename T>
void DartReturn(T result, Dart_NativeArguments args) {
  DartConverter<T>::SetReturnValue(args, std::move(result));
}

template <typename IndicesType, typename T>
class DartDispatcher {};

// void f(ArgTypes...)
template <size_t... indices, typename... ArgTypes>
struct DartDispatcher<IndicesHolder<indices...>, void (*)(ArgTypes...)>
    : public DartArgHolder<indices, ArgTypes>... {
  using FunctionPtr = void (*)(ArgTypes...);

  DartArgIterator* it_;

  explicit DartDispatcher(DartArgIterator* it)
      : DartArgHolder<indices, ArgTypes>(it)..., it_(it) {}

  void Dispatch(FunctionPtr func) {
    (*func)(DartArgHolder<indices, ArgTypes>::value...);
  }
};

// ResultType f(ArgTypes...)
template <size_t... indices, typename ResultType, typename... ArgTypes>
struct DartDispatcher<IndicesHolder<indices...>, ResultType (*)(ArgTypes...)>
    : public DartArgHolder<indices, ArgTypes>... {
  using FunctionPtr = ResultType (*)(ArgTypes...);
  using CtorResultType = ResultType;

  DartArgIterator* it_;

  explicit DartDispatcher(DartArgIterator* it)
      : DartArgHolder<indices, ArgTypes>(it)..., it_(it) {}

  void Dispatch(FunctionPtr func) {
    DartReturn((*func)(DartArgHolder<indices, ArgTypes>::value...),
               it_->args());
  }

  ResultType DispatchCtor(FunctionPtr func) {
    return (*func)(DartArgHolder<indices, ArgTypes>::value...);
  }
};

// void C::m(ArgTypes...)
template <size_t... indices, typename C, typename... ArgTypes>
struct DartDispatcher<IndicesHolder<indices...>, void (C::*)(ArgTypes...)>
    : public DartArgHolder<indices, ArgTypes>... {
  using FunctionPtr = void (C::*)(ArgTypes...);

  DartArgIterator* it_;

  explicit DartDispatcher(DartArgIterator* it)
      : DartArgHolder<indices, ArgTypes>(it)..., it_(it) {}

  void Dispatch(FunctionPtr func) {
    (GetReceiver<C>(it_->args())->*func)(
        DartArgHolder<indices, ArgTypes>::value...);
  }
};

// ReturnType (C::m)(ArgTypes...) const
template <size_t... indices,
          typename C,
          typename ReturnType,
          typename... ArgTypes>
struct DartDispatcher<IndicesHolder<indices...>,
                      ReturnType (C::*)(ArgTypes...) const>
    : public DartArgHolder<indices, ArgTypes>... {
  using FunctionPtr = ReturnType (C::*)(ArgTypes...) const;

  DartArgIterator* it_;

  explicit DartDispatcher(DartArgIterator* it)
      : DartArgHolder<indices, ArgTypes>(it)..., it_(it) {}

  void Dispatch(FunctionPtr func) {
    DartReturn((GetReceiver<C>(it_->args())->*func)(
                   DartArgHolder<indices, ArgTypes>::value...),
               it_->args());
  }
};

// ReturnType (C::m)(ArgTypes...)
template <size_t... indices,
          typename C,
          typename ResultType,
          typename... ArgTypes>
struct DartDispatcher<IndicesHolder<indices...>, ResultType (C::*)(ArgTypes...)>
    : public DartArgHolder<indices, ArgTypes>... {
  using FunctionPtr = ResultType (C::*)(ArgTypes...);

  DartArgIterator* it_;

  explicit DartDispatcher(DartArgIterator* it)
      : DartArgHolder<indices, ArgTypes>(it)..., it_(it) {}

  void Dispatch(FunctionPtr func) {
    DartReturn((GetReceiver<C>(it_->args())->*func)(
                   DartArgHolder<indices, ArgTypes>::value...),
               it_->args());
  }
};

// external static DT FUNCTION(Pointer, ...);
intptr_t FormatFfiNativeFunction(char* buf,
                                 intptr_t buf_size,
                                 const char* ret_type,
                                 const char* name,
                                 bool has_self,
                                 size_t n_args,
                                 va_list args);

std::unique_ptr<char[]> FormatFfiNativeFunction(const char* ret_type,
                                                const char* name,
                                                bool has_self,
                                                size_t n_args,
                                                ...);

// Template voodoo to automatically setup static entry points for FFI Native
// functions.
// Entry points for instance methods take the instance as the first argument and
// call the given method with the remaining arguments.
// Arguments will automatically get converted to and from their FFI
// representations with the DartConverter templates.
template <typename Object, typename Signature, Signature Method>
struct FfiDispatcher;

template <typename Arg, typename... Args>
void print_args(std::ostringstream* stream) {
  *stream << tonic::DartConverter<typename std::remove_const<
      typename std::remove_reference<Arg>::type>::type>::ToFfiSig();
  if constexpr (sizeof...(Args) > 0) {
    *stream << ", ";
    print_args<Args...>(stream);
  }
}

template <typename Arg, typename... Args>
void print_dart_args(std::ostringstream* stream) {
  *stream << tonic::DartConverter<typename std::remove_const<
      typename std::remove_reference<Arg>::type>::type>::ToDartSig();
  if constexpr (sizeof...(Args) > 0) {
    *stream << ", ";
    print_dart_args<Args...>(stream);
  }
}

template <typename Arg, typename... Args>
bool AllowedInLeaf() {
  bool result = tonic::DartConverter<typename std::remove_const<
      typename std::remove_reference<Arg>::type>::type>::AllowedInLeaf();
  if constexpr (sizeof...(Args) > 0) {
    result &= AllowedInLeaf<Args...>();
  }
  return result;
}

// Match `Return Function(...)`.
template <typename Return, typename... Args, Return (*Function)(Args...)>
struct FfiDispatcher<void, Return (*)(Args...), Function> {
  using FfiReturn = typename DartConverter<Return>::FfiType;
  static const size_t n_args = sizeof...(Args);

  // Static C entry-point with Dart FFI signature.
  static FfiReturn Call(
      typename DartConverter<typename std::remove_const<
          typename std::remove_reference<Args>::type>::type>::FfiType... args) {
    // Call C++ function, forwarding converted native arguments.
    return DartConverter<Return>::ToFfi(Function(
        DartConverter<typename std::remove_const<typename std::remove_reference<
            Args>::type>::type>::FromFfi(args)...));
  }
};

// Match `Return Object::Method(...)`.
template <typename Object,
          typename Return,
          typename... Args,
          Return (Object::*Method)(Args...)>
struct FfiDispatcher<Object, Return (Object::*)(Args...), Method> {
  using FfiReturn = typename DartConverter<Return>::FfiType;
  static const size_t n_args = sizeof...(Args);

  // Static C entry-point with Dart FFI signature.
  static FfiReturn Call(
      Object* obj,
      typename DartConverter<typename std::remove_const<
          typename std::remove_reference<Args>::type>::type>::FfiType... args) {
    // Call C++ method on obj, forwarding converted native arguments.
    return DartConverter<Return>::ToFfi((obj->*Method)(
        DartConverter<typename std::remove_const<typename std::remove_reference<
            Args>::type>::type>::FromFfi(args)...));
  }
};

// Match `Return Object::Method(...) const`.
template <typename Object,
          typename Return,
          typename... Args,
          Return (Object::*Method)(Args...) const>
struct FfiDispatcher<Object, Return (Object::*)(Args...) const, Method> {
  using FfiReturn = typename DartConverter<Return>::FfiType;
  static const size_t n_args = sizeof...(Args);

  // Static C entry-point with Dart FFI signature.
  static FfiReturn Call(
      Object* obj,
      typename DartConverter<typename std::remove_const<
          typename std::remove_reference<Args>::type>::type>::FfiType... args) {
    // Call C++ method on obj, forwarding converted native arguments.
    return DartConverter<Return>::ToFfi((obj->*Method)(
        DartConverter<typename std::remove_const<typename std::remove_reference<
            Args>::type>::type>::FromFfi(args)...));
  }
};

// `void` specialisation since we can't declare `ToFfi` to take void rvalues.
// Match `void Function(...)`.
template <typename... Args, void (*Function)(Args...)>
struct FfiDispatcher<void, void (*)(Args...), Function> {
  static const size_t n_args = sizeof...(Args);

  // Static C entry-point with Dart FFI signature.
  static void Call(
      typename DartConverter<typename std::remove_const<
          typename std::remove_reference<Args>::type>::type>::FfiType... args) {
    // Call C++ function, forwarding converted native arguments.
    Function(
        DartConverter<typename std::remove_const<typename std::remove_reference<
            Args>::type>::type>::FromFfi(args)...);
  }
};

// `void` specialisation since we can't declare `ToFfi` to take void rvalues.
// Match `void Object::Method(...)`.
template <typename Object, typename... Args, void (Object::*Method)(Args...)>
struct FfiDispatcher<Object, void (Object::*)(Args...), Method> {
  static const size_t n_args = sizeof...(Args);

  // Static C entry-point with Dart FFI signature.
  static void Call(
      Object* obj,
      typename DartConverter<typename std::remove_const<
          typename std::remove_reference<Args>::type>::type>::FfiType... args) {
    // Call C++ method on obj, forwarding converted native arguments.
    (obj->*Method)(
        DartConverter<typename std::remove_const<typename std::remove_reference<
            Args>::type>::type>::FromFfi(args)...);
  }
};

template <typename Sig>
void DartCall(Sig func, Dart_NativeArguments args) {
  DartArgIterator it(args);
  using Indices = typename IndicesForSignature<Sig>::type;
  DartDispatcher<Indices, Sig> decoder(&it);
  if (it.had_exception())
    return;
  decoder.Dispatch(func);
}

template <typename Sig>
void DartCallStatic(Sig func, Dart_NativeArguments args) {
  DartArgIterator it(args, 0);
  using Indices = typename IndicesForSignature<Sig>::type;
  DartDispatcher<Indices, Sig> decoder(&it);
  if (it.had_exception())
    return;
  decoder.Dispatch(func);
}

template <typename Sig>
void DartCallConstructor(Sig func, Dart_NativeArguments args) {
  DartArgIterator it(args);
  using Indices = typename IndicesForSignature<Sig>::type;
  using Wrappable = typename DartDispatcher<Indices, Sig>::CtorResultType;
  // Call native Create function to construct native obj.
  Wrappable wrappable;
  {
    DartDispatcher<Indices, Sig> decoder(&it);
    if (it.had_exception())
      return;
    wrappable = decoder.DispatchCtor(func);
  }

  // Get first arg which is the Handle for the Dart object being constructed.
  Dart_Handle wrapper = Dart_GetNativeArgument(args, 0);
  TONIC_CHECK(!LogIfError(wrapper));

  // There's only one field which is the native object reference.
  intptr_t native_fields[DartWrappable::kNumberOfNativeFields];
  // Copy native field into array.
  TONIC_CHECK(!LogIfError(Dart_GetNativeFieldsOfArgument(
      args, 0, DartWrappable::kNumberOfNativeFields, native_fields)));
  // Check the native reference .. is null?
  TONIC_CHECK(!native_fields[DartWrappable::kPeerIndex]);

  wrappable->AssociateWithDartWrapper(wrapper);
}

}  // namespace tonic

#endif  // LIB_TONIC_DART_ARGS_H_
