/*
 * Copyright 2017 Google Inc.
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

#ifndef LIB_TXT_SRC_LINE_METRICS_H_
#define LIB_TXT_SRC_LINE_METRICS_H_

#include <map>
#include <vector>

#include "run_metrics.h"

namespace txt {

class LineMetrics {
 public:
  SkRect bounds;               // height, width, startX, endX, offset can
                               // all be derived from this.
  std::vector<uint16_t> text;  // The text within this line.
  double baseline;             // The y position of the baseline for
                               //  this line from the top of the paragraph.
  int line_number;             // The line number where the first is 0

  // Mapping between text position ranges and the FontMetrics
  // associated with them.
  std::map<Range, RunMetrics> fontMetrics;

  LineMetrics();
};

}  // namespace txt

#endif  // LIB_TXT_SRC_LINE_METRICS_H_
