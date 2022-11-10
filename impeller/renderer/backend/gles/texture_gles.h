// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/gles/handle_gles.h"
#include "impeller/renderer/backend/gles/reactor_gles.h"
#include "impeller/renderer/texture.h"

namespace impeller {

enum class TextureBackingTypeGLES {
  kAllocatedTexture,
  kWrappedTexture,
};

struct AllocatedTextureInfoGLES {
  HandleGLES handle;
  bool is_default_fbo = false;
};

struct WrappedTextureInfoGLES {
  GLenum target;
  GLuint name;
};

struct TextureInfoGLES {
  TextureBackingTypeGLES backing_type;
  union {
    WrappedTextureInfoGLES wrapped_texture;
    AllocatedTextureInfoGLES allocated_texture;
  };
};

class TextureGLES final : public Texture,
                          public BackendCast<TextureGLES, Texture> {
 public:
  enum class Type {
    kTexture,
    kRenderBuffer,
  };

  TextureGLES(ReactorGLES::Ref reactor,
              TextureDescriptor desc,
              bool is_default_fbo = false);

  TextureGLES(std::shared_ptr<ReactorGLES> reactor,
              TextureDescriptor desc,
              WrappedTextureInfoGLES wrapped_texture_info);

  // |Texture|
  ~TextureGLES() override;

  std::optional<GLuint> GetGLHandle() const;

  std::optional<GLenum> GetTarget() const;

  [[nodiscard]] bool Bind() const;

  [[nodiscard]] bool GenerateMipmaps() const;

  enum class AttachmentPoint {
    kColor0,
    kDepth,
    kStencil,
  };
  [[nodiscard]] bool SetAsFramebufferAttachment(GLenum target,
                                                GLuint fbo,
                                                AttachmentPoint point) const;

  Type GetType() const;

  bool IsDefaultFBO() const {
    return texture_info_->backing_type ==
               TextureBackingTypeGLES::kAllocatedTexture &&
           texture_info_->allocated_texture.is_default_fbo;
  }

  bool IsExternalTexture() const override {
    return texture_info_->backing_type ==
               TextureBackingTypeGLES::kWrappedTexture &&
           texture_info_->wrapped_texture.target == GL_TEXTURE_EXTERNAL_OES;
  }

 private:
  ReactorGLES::Ref reactor_;
  const Type type_;
  mutable bool contents_initialized_ = false;
  bool is_valid_ = false;
  std::unique_ptr<TextureInfoGLES> texture_info_;

  TextureGLES(std::shared_ptr<ReactorGLES> reactor,
              TextureDescriptor desc,
              std::unique_ptr<TextureInfoGLES> texture_info);

  // |Texture|
  void SetLabel(std::string_view label) override;

  // |Texture|
  bool OnSetContents(const uint8_t* contents,
                     size_t length,
                     size_t slice) override;

  // |Texture|
  bool OnSetContents(std::shared_ptr<const fml::Mapping> mapping,
                     size_t slice) override;

  // |Texture|
  bool IsValid() const override;

  // |Texture|
  ISize GetSize() const override;

  // |Texture|
  Scalar GetYCoordScale() const override;

  void InitializeContentsIfNecessary() const;

  FML_DISALLOW_COPY_AND_ASSIGN(TextureGLES);
};

}  // namespace impeller
