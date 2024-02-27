// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_VISITOR_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_VISITOR_H_

namespace impeller {
// Forward declaration to avoid cyclical includes.
class AnonymousContents;
class AtlasColorContents;
class AtlasContents;
class AtlasTextureContents;
class BlendFilterContents;
class BorderMaskBlurFilterContents;
class CheckerboardContents;
class ClipContents;
class ClipRestoreContents;
class ColorMatrixFilterContents;
class ConicalGradientContents;
class DirectionalMorphologyFilterContents;
class FramebufferBlendContents;
class GaussianBlurFilterContents;
class LinearGradientContents;
class LinearToSrgbFilterContents;
class LocalMatrixFilterContents;
class MatrixFilterContents;
class RadialGradientContents;
class RuntimeEffectContents;
class SceneContents;
class SolidColorContents;
class SolidRRectBlurContents;
class SrgbToLinearFilterContents;
class SweepGradientContents;
class TextContents;
class TextureContents;
class TiledTextureContents;
class VerticesColorContents;
class VerticesContents;
class VerticesUVContents;
class YUVToRGBFilterContents;

class ContentsVisitor {
 public:
  virtual void Visit(AnonymousContents* contents) = 0;
  virtual void Visit(AtlasColorContents* contents) = 0;
  virtual void Visit(AtlasContents* contents) = 0;
  virtual void Visit(AtlasTextureContents* contents) = 0;
  virtual void Visit(BlendFilterContents* contents) = 0;
  virtual void Visit(BorderMaskBlurFilterContents* contents) = 0;
  virtual void Visit(CheckerboardContents* contents) = 0;
  virtual void Visit(ClipContents* contents) = 0;
  virtual void Visit(ClipRestoreContents* contents) = 0;
  virtual void Visit(ColorMatrixFilterContents* contents) = 0;
  virtual void Visit(ConicalGradientContents* contents) = 0;
  virtual void Visit(DirectionalMorphologyFilterContents* contents) = 0;
  virtual void Visit(FramebufferBlendContents* contents) = 0;
  virtual void Visit(GaussianBlurFilterContents* contents) = 0;
  virtual void Visit(LinearGradientContents* contents) = 0;
  virtual void Visit(LinearToSrgbFilterContents* contents) = 0;
  virtual void Visit(LocalMatrixFilterContents* contents) = 0;
  virtual void Visit(MatrixFilterContents* contents) = 0;
  virtual void Visit(RadialGradientContents* contents) = 0;
  virtual void Visit(RuntimeEffectContents* contents) = 0;
  virtual void Visit(SceneContents* contents) = 0;
  virtual void Visit(SolidColorContents* contents) = 0;
  virtual void Visit(SolidRRectBlurContents* contents) = 0;
  virtual void Visit(SrgbToLinearFilterContents* contents) = 0;
  virtual void Visit(SweepGradientContents* contents) = 0;
  virtual void Visit(TextContents* contents) = 0;
  virtual void Visit(TextureContents* contents) = 0;
  virtual void Visit(TiledTextureContents* contents) = 0;
  virtual void Visit(VerticesColorContents* contents) = 0;
  virtual void Visit(VerticesContents* contents) = 0;
  virtual void Visit(VerticesUVContents* contents) = 0;
  virtual void Visit(YUVToRGBFilterContents* contents) = 0;
};

}  // namespace impeller
#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_VISITOR_H_
