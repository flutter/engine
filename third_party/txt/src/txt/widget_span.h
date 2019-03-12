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

#ifndef LIB_TXT_SRC_WIDGET_SPAN_H_
#define LIB_TXT_SRC_WIDGET_SPAN_H_

namespace txt {

// This holds the size
class WidgetSpan {
 public:
  double width = 0;
  double height = 0;
  double baseline = 0;
  bool break_left = true;
  bool break_right = true;

  WidgetSpan();

  WidgetSpan(double width,
             double height,
             double baseline,
             double break_left,
             bool break_right);
};

}  // namespace txt

#endif  // LIB_TXT_SRC_WIDGET_SPAN_H_
