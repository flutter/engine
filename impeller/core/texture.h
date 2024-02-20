// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_CORE_TEXTURE_H_
#define FLUTTER_IMPELLER_CORE_TEXTURE_H_

#include <string_view>

#include "flutter/fml/mapping.h"
#include "impeller/core/buffer_view.h"
#include "impeller/core/formats.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/geometry/size.h"

namespace impeller {

class Texture {
 public:
  virtual ~Texture();

  virtual void SetLabel(std::string_view label) = 0;

  [[nodiscard]] bool SetContents(const uint8_t* contents,
                                 size_t length,
                                 std::optional<IRect> region = std::nullopt,
                                 size_t slice = 0,
                                 bool is_opaque = false);

  /// Set the contents of this texture with new data.
  ///
  /// [region]  The region specifies an area of the destination texture in
  ///           pixels to replace. If not provided, this defaults to the entire
  ///           texture.
  ///
  ///           If a region smaller than the texture size is provided, the
  ///           contents are treated as containing tightly packed pixel data of
  ///           that region. Only the portion of the texture in this region is
  ///           replaced and existing data is preserved.
  ///
  ///           For example, to replace the top left 10 x 10 region of a larger
  ///           100 x 100 texture, the region is {0, 0, 10, 10} and the expected
  ///           buffer size in bytes is 100 x bpp.
  [[nodiscard]] bool SetContents(const BufferView& buffer_view,
                                 std::optional<IRect> region = std::nullopt,
                                 size_t slice = 0,
                                 bool is_opaque = false);

  struct ContentUpdate {
    IRect region;
    BufferView buffer_view;
  };

  [[nodiscard]] bool SetContents(const ContentUpdate updates[],
                                 size_t update_count,
                                 size_t slice = 0,
                                 bool is_opaque = false);

  virtual bool IsValid() const = 0;

  virtual ISize GetSize() const = 0;

  bool IsOpaque() const;

  size_t GetMipCount() const;

  const TextureDescriptor& GetTextureDescriptor() const;

  void SetCoordinateSystem(TextureCoordinateSystem coordinate_system);

  TextureCoordinateSystem GetCoordinateSystem() const;

  virtual Scalar GetYCoordScale() const;

  /// Returns true if mipmaps have never been generated.
  /// The contents of the mipmap may be out of date if the root texture has been
  /// modified and the mipmaps hasn't been regenerated.
  bool NeedsMipmapGeneration() const;

 protected:
  explicit Texture(TextureDescriptor desc);

  [[nodiscard]] virtual bool OnSetContents(const uint8_t* contents,
                                           size_t length,
                                           IRect region,
                                           size_t slice) = 0;

  [[nodiscard]] virtual bool OnSetContents(const BufferView& buffer_view,
                                           IRect region,
                                           size_t slice) = 0;

  [[nodiscard]] virtual bool OnSetContents(const ContentUpdate updates[],
                                           size_t update_count,
                                           size_t slice) {
    return false;
  }

  bool mipmap_generated_ = false;

 private:
  TextureCoordinateSystem coordinate_system_ =
      TextureCoordinateSystem::kRenderToTexture;
  const TextureDescriptor desc_;
  bool is_opaque_ = false;

  bool IsSliceValid(size_t slice) const;

  Texture(const Texture&) = delete;

  Texture& operator=(const Texture&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_CORE_TEXTURE_H_
