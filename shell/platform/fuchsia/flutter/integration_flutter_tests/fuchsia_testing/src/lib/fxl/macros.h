// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FXL_MACROS_H_
#define LIB_FXL_MACROS_H_

#define FXL_DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete

#define FXL_DISALLOW_ASSIGN(TypeName) TypeName& operator=(const TypeName&) = delete

#define FXL_DISALLOW_MOVE(TypeName) \
  TypeName(TypeName&&) = delete;    \
  TypeName& operator=(TypeName&&) = delete

#define FXL_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;          \
  TypeName& operator=(const TypeName&) = delete

#define FXL_DISALLOW_COPY_ASSIGN_AND_MOVE(TypeName) \
  TypeName(const TypeName&) = delete;               \
  TypeName(TypeName&&) = delete;                    \
  TypeName& operator=(const TypeName&) = delete;    \
  TypeName& operator=(TypeName&&) = delete

#define FXL_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                               \
  FXL_DISALLOW_COPY_ASSIGN_AND_MOVE(TypeName)

#endif  // LIB_FXL_MACROS_H_
