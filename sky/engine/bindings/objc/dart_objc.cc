// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/bindings/objc/dart_objc.h"

#include <objc/message.h>
#include <objc/runtime.h>

#include "sky/engine/tonic/dart_args.h"
#include "sky/engine/tonic/dart_binding_macros.h"
#include "sky/engine/tonic/dart_converter.h"
#include "sky/engine/tonic/dart_direct_wrappable.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_library_natives.h"
#include "sky/engine/tonic/dart_wrappable.h"

namespace blink {

IMPLEMENT_DIRECT_WRAPPABLE(Selector, SEL)
IMPLEMENT_DIRECT_WRAPPABLE(Ivar, Ivar)
IMPLEMENT_DIRECT_WRAPPABLE(Property, objc_property_t)
IMPLEMENT_DIRECT_WRAPPABLE(Method, Method)

namespace {

static DartLibraryNatives* g_natives;

Dart_NativeFunction GetNativeFunction(Dart_Handle name,
                                         int argument_count,
                                         bool* auto_setup_scope) {
  return g_natives->GetNativeFunction(name, argument_count, auto_setup_scope);
}

const uint8_t* GetSymbol(Dart_NativeFunction native_function) {
  return g_natives->GetSymbol(native_function);
}

#define DART_OBJC_CALLBACK(CLASS, METHOD) \
  static void Dart_##CLASS##_##METHOD(Dart_NativeArguments args) { \
    DartCall(&CLASS##_##METHOD, args); \
  }

#define DART_REGISTER_OBJC(CLASS, METHOD) \
  { #CLASS "_" #METHOD, Dart_##CLASS##_##METHOD, \
    IndicesForSignature<decltype(&Dart_##CLASS##_##METHOD)>::count + 1, true },

#define FOR_EACH_BINDING(V) \
  V(ivar, getName) \
  V(ivar, getOffset) \
  V(ivar, getTypeEncoding) \
  V(method, getName) \
  V(method, getNumberOfArguments) \
  V(method, getTypeEncoding) \
  V(property, getAttributes) \
  V(property, getName) \
  V(sel, getName) \
  V(sel, getUid) \
  V(sel, isEqual) \
  V(sel, isMapped) \
  V(sel, registerName)

FOR_EACH_BINDING(DART_OBJC_CALLBACK)

}  // namespace

void DartObjC::InitForGlobal() {
  if (!g_natives) {
    g_natives = new DartLibraryNatives();
    g_natives->Register({
FOR_EACH_BINDING(DART_REGISTER_OBJC)
    });
  }
}

void DartObjC::InitForIsolate() {
  DCHECK(g_natives);
  DART_CHECK_VALID(Dart_SetNativeResolver(
      Dart_LookupLibrary(ToDart("dart:objc")), GetNativeFunction, GetSymbol));
}

}  // namespace blink
