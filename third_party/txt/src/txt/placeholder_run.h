/*
 * Copyright 2019 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIB_TXT_SRC_PLACEHOLDER_RUN_H_
#define LIB_TXT_SRC_PLACEHOLDER_RUN_H_

namespace txt {

// Represents the metrics required to fully define a rect that will fit a
// placeholder.
//
// LibTxt will leave an empty space in the layout of the text of the size
// defined by this class. After layout, the framework will draw placeholders
// into the reserved space.
class PlaceholderRun {
 public:
  double width = 0;
  double height = 0;
  // Distance from the top edge of the rect to the baseline position. This
  // baseline will be aligned against the alphabetic baseline of the surrounding
  // text.
  //
  // Positive values drop the baseline lower (positions the rect higher) and
  // small or negative values will cause the rect to be positioned underneath
  // the line. When baseline == height, the bottom edge of the rect will rest on
  // the alphabetic baseline.
  double baseline = 0;

  PlaceholderRun();

  PlaceholderRun(double width, double height, double baseline);
};

}  // namespace txt

#endif  // LIB_TXT_SRC_PLACEHOLDER_RUN_H_
