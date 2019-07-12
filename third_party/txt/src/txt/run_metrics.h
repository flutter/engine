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

#ifndef LIB_TXT_SRC_RUN_METRICS_H_
#define LIB_TXT_SRC_RUN_METRICS_H_

#include "text_style.h"
#include "third_party/skia/include/core/SkFontMetrics.h"

namespace txt {

class RunMetrics {
 public:
  TextStyle* text_style = nullptr;

  // double Top;                 // distance to reserve above baseline
  // double Ascent;              // distance to reserve below baseline
  // double Descent;             // extent below baseline
  // double Bottom;              // extent below baseline
  // double Leading;             // distance to add between lines
  // double AvgCharWidth;        // average character width
  // double MaxCharWidth;        // maximum character width
  // double XMin;                // minimum x
  // double XMax;                // maximum x
  // double XHeight;             // height of lower-case 'x'
  // double CapHeight;           // height of an upper-case letter
  // double UnderlineThickness;  // underline thickness
  // double UnderlinePosition;   // underline position relative to baseline
  // double StrikeoutThickness;  // strikeout thickness
  // double StrikeoutPosition;   // strikeout position relative to baseline
  SkFontMetrics* font_metrics = nullptr;

  RunMetrics();

  RunMetrics(TextStyle* style, SkFontMetrics* metrics)
      : text_style(style), font_metrics(metrics) {}
};

}  // namespace txt

#endif  // LIB_TXT_SRC_RUN_METRICS_H_
