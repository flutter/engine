// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_SEMANTICS_ATTRIBUTED_STRING_H_
#define FLUTTER_LIB_UI_SEMANTICS_ATTRIBUTED_STRING_H_

#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/semantics/string_attribute.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

class SemanticsUpdateBuilder;

//------------------------------------------------------------------------------
/// The peer class for the AttributedString in semantics.dart.
class AttributedString : public RefCountedDartWrappable<AttributedString> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(AttributedString);

 public:
  ~AttributedString() override;
  //----------------------------------------------------------------------------
  /// The init method for constructor
  /// AttributedString(String, List<StringAttribute>?).
  static void initAttributedString(
      Dart_Handle attributed_string_handle,
      std::string string,
      std::vector<NativeStringAttribute*> attributes);

  //----------------------------------------------------------------------------
  /// Gets the plain string that are stored in this attributed string.
  const std::string& GetString();

  //----------------------------------------------------------------------------
  /// Gets attributes that this attributed string carries.
  const StringAttributes& GetAttributes();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  AttributedString();
  std::string string_;
  StringAttributes attributes_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_SEMANTICS_ATTRIBUTED_STRING_H_
