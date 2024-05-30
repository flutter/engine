// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_WEB_UI_SKWASM_IMPELLER_STRATEGY_H_
#define FLUTTER_LIB_WEB_UI_SKWASM_IMPELLER_STRATEGY_H_

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
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/effects/SkImageFilters.h"
#include "third_party/skia/include/effects/SkRuntimeEffect.h"
#include "third_party/skia/include/pathops/SkPathOps.h"
#include "third_party/skia/include/ports/SkFontMgr_empty.h"
#include "third_party/skia/include/utils/SkShadowUtils.h"
#include "third_party/skia/modules/skparagraph/include/FontCollection.h"
#include "third_party/skia/modules/skparagraph/include/Paragraph.h"
#include "third_party/skia/modules/skparagraph/include/TypefaceFontProvider.h"

#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_builder.h"
#include "flutter/display_list/dl_canvas.h"
#include "flutter/display_list/dl_color.h"
#include "flutter/display_list/dl_paint.h"
#include "flutter/display_list/skia/dl_sk_dispatcher.h"

#include <emscripten/html5_webgl.h>

class GrTextureGenerator;
namespace impeller {
    class ContextGLES;
    class Surface;
    class Picture;
    class Renderer;
}

namespace Skwasm {
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
    using ImageFilter = SkImageFilter;
    using ImageFilters = SkImageFilters;
    using ImageInfo = SkImageInfo;
    using IRect = SkIRect;
    using MaskFilter = SkMaskFilter;
    using Matrix = SkMatrix;
    using Matrix44 = SkM44;
    using MipmapMode = SkMipmapMode;
    using Paragraph = skia::textlayout::Paragraph;
    using Path = SkPath;
    using PathFillType = SkPathFillType;
    using PathDirection = SkPathDirection;
    using PathOp = SkPathOp;
    using Point = SkPoint;
    using Point3 = SkPoint3;
    using Rect = SkRect;
    using RRect = SkRRect;
    using RSXform = SkRSXform;
    using RuntimeEffect = SkRuntimeEffect;
    using SamplingOptions = SkSamplingOptions;
    using Scalar = SkScalar;
    using Shader = SkShader;
    using ShadowFlags = SkShadowFlags;
    using ShadowUtils = SkShadowUtils;
    using TileMode = SkTileMode;
    using Vector = SkVector;
    using Vertices = SkVertices;

    using BlendMode = flutter::DlBlendMode;
    using ClipOp = flutter::DlCanvas::ClipOp;

    class Paint : public flutter::DlPaint {
    public:
        using Style = flutter::DlDrawStyle;
        using Cap = flutter::DlStrokeCap;
        using Join = flutter::DlStrokeJoin;

        void setStyle(Style style) {
            setDrawStyle(style);
        }

        Style getStyle() const {
            return getDrawStyle();
        }

        Color getColor() const {
            return flutter::DlPaint::getColor().argb();
        }

        void setColor(Color color) {
            flutter::DlPaint::setColor(flutter::DlColor(color));
        }

        void setShader(const sk_sp<SkShader>& shader) {
            // TODO: implement this
        }
    };

    class RTreeFactory {};

    class Picture : public flutter::DisplayList {
    public:
        Rect cullRect() {
            return bounds();
        }

        int approximateBytesUsed() {
            return bytes();
        }
    };

    class Canvas {
    public:
        using PointMode = flutter::DlCanvas::PointMode;

        Canvas() {
        }

        void save() {
            _builder.Save();
        }

        struct SaveLayerRec {
            SaveLayerRec(Rect *rect, Paint* paint, ImageFilter* backdrop, int flags) 
            : rect(rect),
              paint(paint),
              backdrop(backdrop) {}
            Rect* rect;
            Paint* paint;
            ImageFilter* backdrop;
        };

        void saveLayer(SaveLayerRec rec) {
            // TODO: pass backdrop
            _builder.SaveLayer(rec.rect, rec.paint, nullptr);
        }

        void restore() {
            _builder.Restore();
        }

        void restoreToCount(int count) {
            _builder.RestoreToCount(count);
        }

        int getSaveCount() {
            return _builder.GetSaveCount();
        }

        void translate(Scalar dx, Scalar dy) {
            _builder.Translate(dx, dy);
        }

        void scale(Scalar sx, Scalar sy) {
            _builder.Scale(sx, sy);
        }

        void rotate(Scalar degrees) {
            _builder.Rotate(degrees);
        }

        void skew(Scalar sx, Scalar sy) {
            _builder.Skew(sx, sy);
        }

        void concat(const Matrix44& matrix44) {
            _builder.Transform(matrix44);
        }

        void clipRect(const Rect& rect, ClipOp op, bool antialias) {
            _builder.ClipRect(rect, op, antialias);
        }

        void clipRRect(const RRect& rrect, bool antialias) {
            _builder.ClipRRect(rrect, ClipOp::kIntersect, antialias);
        }

        void clipPath(const Path& path, bool antialias) {
            _builder.ClipPath(path, ClipOp::kIntersect, antialias);
        }

        void drawColor(Color color, BlendMode blendMode) {
            _builder.DrawColor(flutter::DlColor(color), blendMode);
        }

        void drawLine(Scalar x1, Scalar y1, Scalar x2, Scalar y2, const Paint& paint) {
            _builder.DrawLine({x1, y1}, {x2, y2}, paint);
        }

        void drawPaint(const Paint& paint) {
            _builder.DrawPaint(paint);
        }

        void drawRect(const Rect& rect, const Paint& paint) {
            _builder.DrawRect(rect, paint);
        }

        void drawRRect(const RRect &rrect, const Paint& paint) {
            _builder.DrawRRect(rrect, paint);
        }

        void drawDRRect(const RRect &outer, const RRect &inner, const Paint& paint) {
            _builder.DrawDRRect(outer, inner, paint);
        }

        void drawOval(const Rect& rect, const Paint& paint) {
            _builder.DrawOval(rect, paint);
        }

        void drawCircle(Scalar x, Scalar y, Scalar radius, const Paint& paint) {
            _builder.DrawCircle({x, y}, radius, paint);
        }

        void drawArc(const Rect& rect, Scalar startAngleDegrees, Scalar sweepAngleDegrees, bool useCenter, const Paint& paint) {
            _builder.DrawArc(rect, startAngleDegrees, sweepAngleDegrees, useCenter, paint);
        }

        void drawPath(const Path& path, const Paint& paint) {
            _builder.DrawPath(path, paint);
        }

        void drawPicture(const Picture* picture) {
            _builder.DrawDisplayList(sk_ref_sp(picture));
        }

        void drawShadow(Path* path,
                        Scalar elevation,
                        Scalar devicePixelRatio,
                        Color color,
                        bool transparentOccluder) {
            _builder.DrawShadow(*path, flutter::DlColor(color), elevation, transparentOccluder, devicePixelRatio);
        }

        void drawImage(Image* image, Scalar offsetX, Scalar offsetY, SamplingOptions quality, Paint* paint) {
            // TODO: draw images
        }

        static constexpr int kStrict_SrcRectConstraint = 0;
        void drawImageRect(Image* image, const Rect& sourceRect, const Rect& destRect, SamplingOptions qualtiy, Paint* paint, int) {
            // TODO: draw images
        }

        void drawImageNine(Image* image, const IRect& centerRect, const Rect& destinationRect, FilterMode quality, Paint* paint) {
            // TODO: draw images
        }

        void drawVertices(const Vertices* vertices, BlendMode mode, const Paint& paint) {
            // TODO: Draw vertices
            //_builder.DrawVertices(vertices, mode, paint);
        }

        void drawPoints(Canvas::PointMode mode,
                        int pointCount,
                        const Point* points,
                        const Paint& paint) {
            _builder.DrawPoints(mode, pointCount, points, paint);
        }

        void drawAtlas(Image* atlas,
                       RSXform* transforms,
                       Rect* rects,
                       Color* colors,
                       int spriteCount,
                       BlendMode mode,
                       SamplingOptions quality,
                       Rect* cullRect,
                       Paint* paint) {
            // TODO: draw atlas
        }

        Matrix44 getLocalToDevice() {
            return _builder.GetTransformFullPerspective();
        }

        Rect getLocalClipBounds() {
            return _builder.GetLocalClipBounds();
        }

        IRect getDeviceClipBounds() {
            return _builder.GetDestinationClipBounds().roundOut();
        }

        sk_sp<Picture> build() {
            return sk_ref_sp<Picture>(static_cast<Picture*>(_builder.Build().release()));
        }

    private:
        flutter::DisplayListBuilder _builder;
    };

    static inline void drawShadowOnCanvas(Canvas* canvas,
                                    Path* path,
                                    Scalar elevation,
                                    Scalar devicePixelRatio,
                                    Color color,
                                    bool transparentOccluder) {
        canvas->drawShadow(path, elevation, devicePixelRatio, color, transparentOccluder);
    }

    static inline void drawParagraphOnCanvas(Canvas* canvas, Paragraph* paragraph, Scalar x, Scalar y) {
        // TODO: Draw paragraphs.
    }

    class PictureRecorder : public SkRefCnt {
    public:
        PictureRecorder() {};

        Canvas *beginRecording(const Rect& cullRect, RTreeFactory* factory) {
            assert(!_canvas);
            _canvas = std::make_unique<Canvas>();
            return _canvas.get();
        }

        sk_sp<Picture> finishRecordingAsPicture() {
            return _canvas->build();
        }
    private:
        std::unique_ptr<Canvas> _canvas;
    };

    namespace Images {
        using BitDepth = SkImages::BitDepth;

        inline sk_sp<SkImage> RasterFromData(const SkImageInfo& info, sk_sp<SkData> data, size_t rowBytes) {
            return nullptr;
        }
        inline sk_sp<SkImage> DeferredFromPicture(sk_sp<Picture> picture,
                                   const SkISize& dimensions,
                                   const SkMatrix* matrix,
                                   const SkPaint* paint,
                                   BitDepth bitDepth,
                                   sk_sp<SkColorSpace> colorSpace) {
            return nullptr;
        }
        inline sk_sp<SkImage> DeferredFromTextureGenerator(std::unique_ptr<GrTextureGenerator> &&generator) {
            return nullptr;
        }
    }

    class GraphicsSurface : public SkRefCnt {
    public:
        GraphicsSurface(
            std::shared_ptr<impeller::ContextGLES> context,
            std::shared_ptr<impeller::Renderer> renderer,
            int width,
            int height);

        void renderPicture(const impeller::Picture& picture);

        ~GraphicsSurface();
    private:
        std::shared_ptr<impeller::ContextGLES> _context;
        std::shared_ptr<impeller::Renderer> _renderer;
        int _width;
        int _height;
    };


    class ReactorWorker;
    class GraphicsContext : public SkRefCnt {
    public:
        GraphicsContext(
            std::shared_ptr<impeller::ContextGLES> context,
            std::shared_ptr<impeller::Renderer> renderer,
            std::shared_ptr<ReactorWorker> worker);

        sk_sp<GraphicsSurface> createSurface(
            int width,
            int height
        );

        void flush(GraphicsSurface* surface) {
        }

    private:
        std::shared_ptr<impeller::ContextGLES> _context;
        std::shared_ptr<impeller::Renderer> _renderer;
        std::shared_ptr<ReactorWorker> _worker;
    };

    sk_sp<GraphicsContext> createGraphicsContext();

    inline sk_sp<GraphicsSurface> createGraphicsSurface(
        const sk_sp<GraphicsContext>& context,
        int width,
        int height,
        int sampleCount,
        int stencil
    ) {
        return context->createSurface(width, height);
    }

    inline void resetGraphicsContext(const sk_sp<GraphicsContext>) {
    }

    void drawPictureToSurface(Picture* picture, GraphicsSurface* surface, Scalar offsetX, Scalar offsetY);
}

#endif  // FLUTTER_LIB_WEB_UI_SKWASM_IMPELLER_STRATEGY_H_
