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

#ifndef LIB_TXT_SRC_STROKE_STYLE_H_
#define LIB_TXT_SRC_STROKE_STYLE_H_

namespace txt {

enum class StrokeStyle {
  // No stroke
  kNone,
  // The stroke is drawn under the fill text. The stroke appears as an
  // "outset" to the fill.
  kUnder,
  // The stroke is drawn over the fill text. The fill text will be
  // partially covered by the stroke.
  kOver,

  // We may support a `kInset` stroke (kOver, clipped by the fill text)
  // in the future.
};

}  // namespace txt

#endif  // LIB_TXT_SRC_STROKE_STYLE_H_
