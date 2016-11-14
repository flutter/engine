// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_PICTURE_H_
#define FLUTTER_LIB_UI_PAINTING_PICTURE_H_

#include "lib/tonic/dart_wrappable.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

class Picture : public ftl::RefCountedThreadSafe<Picture>,
                public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();
  FRIEND_MAKE_REF_COUNTED(Picture);

 public:
  ~Picture() override;
  static ftl::RefPtr<Picture> Create(sk_sp<SkPicture> picture,
                                     SkRect picture_bounds);

  const sk_sp<SkPicture>& picture() const { return picture_; }
  const SkRect& picture_bounds() const { return picture_bounds_; }

  void dispose();

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

 private:
  explicit Picture(sk_sp<SkPicture> picture, SkRect picture_bounds);

  sk_sp<SkPicture> picture_;
  SkRect picture_bounds_;
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_PAINTING_PICTURE_H_
