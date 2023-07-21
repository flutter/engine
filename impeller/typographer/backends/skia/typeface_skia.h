// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/typographer/typeface.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkTypeface.h"

namespace impeller {

class TypefaceSkia final : public Typeface,
                           public BackendCast<TypefaceSkia, Typeface> {
 public:
  TypefaceSkia(SkFont font);

  ~TypefaceSkia() override;

  // |Typeface|
  bool IsValid() const override;

  // |Comparable<Typeface>|
  std::size_t GetHash() const override;

  // |Comparable<Typeface>|
  bool IsEqual(const Typeface& other) const override;

  const SkFont& GetSkiaFont() const;

 private:
  SkFont font_;

  FML_DISALLOW_COPY_AND_ASSIGN(TypefaceSkia);
};

}  // namespace impeller
