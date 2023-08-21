// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/typographer/typeface.h"
#include "third_party/stb/stb_truetype.h"

namespace impeller {

class TypefaceSTB final : public Typeface,
                           public BackendCast<TypefaceSTB, Typeface> {
 public:
  TypefaceSTB() = delete;

  TypefaceSTB(const unsigned char * ttf_buffer, size_t buffer_size);

  ~TypefaceSTB() override;

  // |Typeface|
  bool IsValid() const override;

  // |Comparable<Typeface>|
  std::size_t GetHash() const override;

  // |Comparable<Typeface>|
  bool IsEqual(const Typeface& other) const override;

  const uint8_t* GetTypefaceFile() const;
  const stbtt_fontinfo* GetFontInfo() const;

 private:
  std::unique_ptr<const uint8_t[]> _font_file;
  std::unique_ptr<stbtt_fontinfo> _font_info;
  bool is_valid;

  FML_DISALLOW_COPY_AND_ASSIGN(TypefaceSTB);
};

}  // namespace impeller
