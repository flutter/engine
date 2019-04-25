// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_STRING_RANGE_SANITIZATION_H_
#define FLUTTER_FML_STRING_RANGE_SANITIZATION_H_

#include <Foundation/Foundation.h>

namespace fml {

// Helper to get the correct boundary of a character position in an NSString
// given a byte index.
NSRange RangeForCharacterAtIndex(NSString* text, NSUInteger index);

// Helper to get the correct boundaries around one or more character positions
// in an NSString given an NSRange.
//
// This method will not alter the length of the input range, but will ensure
// that the range's location is not in the middle of a multi-byte unicode
// sequence.
NSRange RangeForCharactersInRange(NSString* text, NSRange range);

}  // namespace fml

#endif  // FLUTTER_FML_STRING_RANGE_SANITIZATION_H_
