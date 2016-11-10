// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart_ui;

class _HashEnd { const _HashEnd(); }
const _HashEnd _hashEnd = const _HashEnd();

/// Combine up to twenty values' hashCodes into one value.
///
/// If you only need to handle one value's hashCode, then just refer to its
/// [hashCode] getter directly.
///
/// If you need to combine an arbitrary number of values from a List or other
/// Iterable, use [hashList]. The output of hashList can be used as one of the
/// arguments to this function.
///
/// For example:
///
///   int hashCode => hashValues(foo, bar, hashList(quux), baz);
int hashValues(
  Object arg01,            Object arg02,          [ Object arg03 = _hashEnd,
  Object arg04 = _hashEnd, Object arg05 = _hashEnd, Object arg06 = _hashEnd,
  Object arg07 = _hashEnd, Object arg08 = _hashEnd, Object arg09 = _hashEnd,
  Object arg10 = _hashEnd, Object arg11 = _hashEnd, Object arg12 = _hashEnd,
  Object arg13 = _hashEnd, Object arg14 = _hashEnd, Object arg15 = _hashEnd,
  Object arg16 = _hashEnd, Object arg17 = _hashEnd, Object arg18 = _hashEnd,
  Object arg19 = _hashEnd, Object arg20 = _hashEnd ]) {
  int result = 373;
  void append(Object o) {
    assert(o is! Iterable);
    result = 0x1fffffff & (result + o.hashCode);
    result = 0x1fffffff & (result + ((0x0007ffff & result) << 10));
    result = result ^ (result >> 6);
  }
  append(arg01);
  append(arg02);
  if (arg03 == _hashEnd) return result;
  append(arg03);
  if (arg04 == _hashEnd) return result;
  append(arg04);
  if (arg05 == _hashEnd) return result;
  append(arg05);
  if (arg06 == _hashEnd) return result;
  append(arg06);
  if (arg07 == _hashEnd) return result;
  append(arg07);
  if (arg08 == _hashEnd) return result;
  append(arg08);
  if (arg09 == _hashEnd) return result;
  append(arg09);
  if (arg10 == _hashEnd) return result;
  append(arg10);
  if (arg11 == _hashEnd) return result;
  append(arg11);
  if (arg12 == _hashEnd) return result;
  append(arg12);
  if (arg13 == _hashEnd) return result;
  append(arg13);
  if (arg14 == _hashEnd) return result;
  append(arg14);
  if (arg15 == _hashEnd) return result;
  append(arg15);
  if (arg16 == _hashEnd) return result;
  append(arg16);
  if (arg17 == _hashEnd) return result;
  append(arg17);
  if (arg18 == _hashEnd) return result;
  append(arg18);
  if (arg19 == _hashEnd) return result;
  append(arg19);
  if (arg20 == _hashEnd) return result;
  append(arg20);
  return result;
}


/// Combine the hashCodes of an arbitrary number of values from an Iterable into
/// one value. This function will return the same value if given "null" as if
/// given an empty list.
int hashList(Iterable<Object> args) {
  int result = 373;
  void append(Object o) {
    assert(o is! Iterable);
    result = 0x1fffffff & (result + o.hashCode);
    result = 0x1fffffff & (result + ((0x0007ffff & result) << 10));
    return result ^ (result >> 6);
  }
  if (args != null) {
    for (Object arg in args) {
      append(arg);
    }
  }
  return result;
}
