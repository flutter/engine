// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_WEB_UI_SKWASM_SKIA_STRATEGY_H_
#define FLUTTER_LIB_WEB_UI_SKWASM_SKIA_STRATEGY_H_

#include "third_party/skia/include/core/SkBBHFactory.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkContourMeasure.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkPoint3.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkString.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkVertices.h"
#include "third_party/skia/include/core/SkSamplingOptions.h"
#include "third_party/skia/include/encode/SkPngEncoder.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "third_party/skia/include/gpu/GpuTypes.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/ganesh/GrExternalTextureGenerator.h"
#include "third_party/skia/include/gpu/ganesh/SkImageGanesh.h"
#include "third_party/skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"
#include "third_party/skia/include/gpu/gl/GrGLTypes.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/effects/SkImageFilters.h"
#include "third_party/skia/include/effects/SkRuntimeEffect.h"
#include "third_party/skia/include/pathops/SkPathOps.h"
#include "third_party/skia/include/ports/SkFontMgr_empty.h"
#include "third_party/skia/include/utils/SkShadowUtils.h"
#include "third_party/skia/modules/skparagraph/include/FontCollection.h"
#include "third_party/skia/modules/skparagraph/include/Paragraph.h"
#include "third_party/skia/modules/skparagraph/include/TypefaceFontProvider.h"

namespace Skwasm {
    using BlendMode = SkBlendMode;
    using Canvas = SkCanvas;
    using ClipOp = SkClipOp;
    using Color = SkColor;
    using ColorFilter = SkColorFilter;
    using ColorFilters = SkColorFilters;
    using ColorSpace = SkColorSpace;
    using ColorType = SkColorType;
    using ContourMeasure = SkContourMeasure;
    using ContourMeasureIter = SkContourMeasureIter;
    using FilterMode = SkFilterMode;
    using GradientShader = SkGradientShader;
    using Image = SkImage;
    namespace Images = SkImages;
    using ImageFilter = SkImageFilter;
    using ImageFilters = SkImageFilters;
    using ImageInfo = SkImageInfo;
    using IRect = SkIRect;
    using MaskFilter = SkMaskFilter;
    using Matrix = SkMatrix;
    using Matrix44 = SkM44;
    using MipmapMode = SkMipmapMode;
    using Paint = SkPaint;
    using Path = SkPath;
    using PathFillType = SkPathFillType;
    using PathDirection = SkPathDirection;
    using PathOp = SkPathOp;
    using Paragraph = skia::textlayout::Paragraph;
    using Picture = SkPicture;
    using PictureRecorder = SkPictureRecorder;
    using Point = SkPoint;
    using Point3 = SkPoint3;
    using Rect = SkRect;
    using RRect = SkRRect;
    using RSXform = SkRSXform;
    using RTreeFactory = SkRTreeFactory;
    using RuntimeEffect = SkRuntimeEffect;
    using SamplingOptions = SkSamplingOptions;
    using Scalar = SkScalar;
    using Shader = SkShader;
    using ShadowFlags = SkShadowFlags;
    using ShadowUtils = SkShadowUtils;
    using TileMode = SkTileMode;
    using Vector = SkVector;
    using Vertices = SkVertices;

    static constexpr int ColorGetAlpha(Color color) { return SkColorGetA(color); }
    static constexpr Color ColorSetAlpha(Color color, U8CPU alpha) { return SkColorSetA(color, alpha); }

    static inline void drawShadowOnCanvas(Canvas* canvas,
                                     Path* path,
                                     Scalar elevation,
                                     Scalar devicePixelRatio,
                                     Color color,
                                     bool transparentOccluder) {
        // These numbers have been chosen empirically to give a result closest to the
        // material spec.
        // These values are also used by the CanvasKit renderer and the native engine.
        // See:
        //   flutter/display_list/skia/dl_sk_dispatcher.cc
        //   flutter/lib/web_ui/lib/src/engine/canvaskit/util.dart
        constexpr Scalar kShadowAmbientAlpha = 0.039;
        constexpr Scalar kShadowSpotAlpha = 0.25;
        constexpr Scalar kShadowLightRadius = 1.1;
        constexpr Scalar kShadowLightHeight = 600.0;
        constexpr Scalar kShadowLightXOffset = 0;
        constexpr Scalar kShadowLightYOffset = -450;

        Color inAmbient =
            ColorSetAlpha(color, kShadowAmbientAlpha * ColorGetAlpha(color));
        Color inSpot = ColorSetAlpha(color, kShadowSpotAlpha * ColorGetAlpha(color));
        Color outAmbient;
        Color outSpot;
        ShadowUtils::ComputeTonalColors(inAmbient, inSpot, &outAmbient, &outSpot);
        uint32_t flags = transparentOccluder
                            ? ShadowFlags::kTransparentOccluder_ShadowFlag
                            : ShadowFlags::kNone_ShadowFlag;
        flags |= ShadowFlags::kDirectionalLight_ShadowFlag;
        ShadowUtils::DrawShadow(
            canvas, *path, Point3::Make(0.0f, 0.0f, elevation * devicePixelRatio),
            Point3::Make(kShadowLightXOffset, kShadowLightYOffset,
                            kShadowLightHeight * devicePixelRatio),
            devicePixelRatio * kShadowLightRadius, outAmbient, outSpot, flags);
    }

    static inline void drawParagraphOnCanvas(Canvas* canvas, Paragraph* paragraph, Scalar x, Scalar y) {
        paragraph->paint(canvas, x, y);
    }

    inline void drawPictureToSurface(Picture* picture, SkSurface* surface, Scalar offsetX, Scalar offsetY) {
        auto canvas = surface->getCanvas();
        canvas->drawColor(SK_ColorTRANSPARENT, BlendMode::kSrc);

        SkMatrix matrix = SkMatrix::Translate(offsetX, offsetY);
        canvas->drawPicture(picture, &matrix, nullptr);
    }
}

#endif  // FLUTTER_LIB_WEB_UI_SKWASM_SKIA_STRATEGY_H_
