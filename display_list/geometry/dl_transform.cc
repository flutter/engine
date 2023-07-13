// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_transform.h"

namespace flutter {

DlTransform::Complexity DlTransform::complexity() const {
  if (complexity_ == Complexity::kUnknown) {
    if (                                    m_[ 2] == 0.0f && m_[ 3] == 0.0f &&
                                            m_[ 6] == 0.0f && m_[ 7] == 0.0f &&
        m_[ 8] == 0.0f && m_[ 9] == 0.0f && m_[10] == 1.0f && m_[11] == 0.0f &&
                                            m_[14] == 0.0f && m_[15] == 1.0f) {

      if (m_[1] == 0.0f && m_[4] == 0.0f) {
        if (m_[0] == 1.0f && m_[5] == 1.0f) {
          if (m_[12] == 0.0f && m_[13] == 0.0f) {
            complexity_ = Complexity::kIdentity;
          } else {
            complexity_ = Complexity::kTranslate;
          }
        } else {
          complexity_ = Complexity::kScaleTranslate;
        }
      } else {
        complexity_ = Complexity::kAffine2D;
      }
    } else {
      complexity_ = Complexity::kComplex;
    }
  }
  return complexity_;
}

DlRectF DlTransform::TransformRect(const DlRectF& rect) const {
  switch
}

}  // namespace flutter
