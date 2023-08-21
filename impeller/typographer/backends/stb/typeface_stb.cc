// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/stb/typeface_stb.h"

#include <cstring>  // memcpy
#include "flutter/fml/logging.h"

namespace impeller {

TypefaceSTB::~TypefaceSTB() = default;

// Instantiate a typeface based on a .ttf or other font file
TypefaceSTB::TypefaceSTB(const unsigned char* ttf_buffer, size_t buffer_size)
    : _font_file(std::make_unique<const uint8_t[]>(buffer_size)),
      _font_info(std::make_unique<stbtt_fontinfo>()),
      is_valid(false) {
  // As we lazily create atlases based on this font, we have to store the binary
  // font file itself This seems memory intensive, so maybe we could improve
  // this in time (extract needed info now e.g.).
  memcpy((void*)_font_file.get(), (void*)ttf_buffer, buffer_size);

  // We need an "offset" into the ttf file
  auto offset = stbtt_GetFontOffsetForIndex(ttf_buffer, 0);
  if (stbtt_InitFont(_font_info.get(), ttf_buffer, offset) == 0) {
    FML_LOG(ERROR) << "Failed to initialize stb font from binary data.";
  } else {
    is_valid = true;
  }
}

bool TypefaceSTB::IsValid() const {
  return is_valid;
}

std::size_t TypefaceSTB::GetHash() const {
  if (!IsValid()) {
    return 0u;
  }
  return reinterpret_cast<size_t>(_font_file.get());
}

bool TypefaceSTB::IsEqual(const Typeface& other) const {
  auto stb_other = reinterpret_cast<const TypefaceSTB*>(&other);
  return stb_other->GetHash() == GetHash();
}

const uint8_t* TypefaceSTB::GetTypefaceFile() const {
  return _font_file.get();
}

const stbtt_fontinfo* TypefaceSTB::GetFontInfo() const {
  return _font_info.get();
}

}  // namespace impeller
