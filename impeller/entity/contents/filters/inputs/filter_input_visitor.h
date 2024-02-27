// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_INPUTS_FILTER_INPUT_VISITOR_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_INPUTS_FILTER_INPUT_VISITOR_H_

namespace impeller {

// Forward declarations to avoid cyclical include.
class ContentsFilterInput;
class FilterContentsFilterInput;
class PlaceholderFilterInput;
class TextureFilterInput;

class FilterInputVisitor {
 public:
  virtual void Visit(ContentsFilterInput* filter_input) = 0;
  virtual void Visit(FilterContentsFilterInput* filter_input) = 0;
  virtual void Visit(PlaceholderFilterInput* filter_input) = 0;
  virtual void Visit(TextureFilterInput* filter_input) = 0;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_INPUTS_FILTER_INPUT_VISITOR_H_
