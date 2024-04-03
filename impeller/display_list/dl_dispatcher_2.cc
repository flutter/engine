// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/dl_dispatcher_2.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "flutter/fml/logging.h"
#include "impeller/aiks/color_filter.h"
#include "impeller/core/formats.h"
#include "impeller/display_list/skia_conversions.h"
#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/entity/contents/filters/inputs/filter_input.h"
#include "impeller/entity/contents/runtime_effect_contents.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/scalar.h"
#include "impeller/geometry/sigma.h"

#if IMPELLER_ENABLE_3D
#include "impeller/entity/contents/scene_contents.h"
#endif  // IMPELLER_ENABLE_3D

namespace impeller {

#define UNIMPLEMENTED \
  FML_DLOG(ERROR) << "Unimplemented detail in " << __FUNCTION__;

DlDispatcher2::DlDispatcher2(ContentContext& renderer,
                             RenderTarget& render_target)
    : canvas_(renderer, render_target) {}

DlDispatcher2::~DlDispatcher2() = default;

static BlendMode ToBlendMode(flutter::DlBlendMode mode) {
  switch (mode) {
    case flutter::DlBlendMode::kClear:
      return BlendMode::kClear;
    case flutter::DlBlendMode::kSrc:
      return BlendMode::kSource;
    case flutter::DlBlendMode::kDst:
      return BlendMode::kDestination;
    case flutter::DlBlendMode::kSrcOver:
      return BlendMode::kSourceOver;
    case flutter::DlBlendMode::kDstOver:
      return BlendMode::kDestinationOver;
    case flutter::DlBlendMode::kSrcIn:
      return BlendMode::kSourceIn;
    case flutter::DlBlendMode::kDstIn:
      return BlendMode::kDestinationIn;
    case flutter::DlBlendMode::kSrcOut:
      return BlendMode::kSourceOut;
    case flutter::DlBlendMode::kDstOut:
      return BlendMode::kDestinationOut;
    case flutter::DlBlendMode::kSrcATop:
      return BlendMode::kSourceATop;
    case flutter::DlBlendMode::kDstATop:
      return BlendMode::kDestinationATop;
    case flutter::DlBlendMode::kXor:
      return BlendMode::kXor;
    case flutter::DlBlendMode::kPlus:
      return BlendMode::kPlus;
    case flutter::DlBlendMode::kModulate:
      return BlendMode::kModulate;
    case flutter::DlBlendMode::kScreen:
      return BlendMode::kScreen;
    case flutter::DlBlendMode::kOverlay:
      return BlendMode::kOverlay;
    case flutter::DlBlendMode::kDarken:
      return BlendMode::kDarken;
    case flutter::DlBlendMode::kLighten:
      return BlendMode::kLighten;
    case flutter::DlBlendMode::kColorDodge:
      return BlendMode::kColorDodge;
    case flutter::DlBlendMode::kColorBurn:
      return BlendMode::kColorBurn;
    case flutter::DlBlendMode::kHardLight:
      return BlendMode::kHardLight;
    case flutter::DlBlendMode::kSoftLight:
      return BlendMode::kSoftLight;
    case flutter::DlBlendMode::kDifference:
      return BlendMode::kDifference;
    case flutter::DlBlendMode::kExclusion:
      return BlendMode::kExclusion;
    case flutter::DlBlendMode::kMultiply:
      return BlendMode::kMultiply;
    case flutter::DlBlendMode::kHue:
      return BlendMode::kHue;
    case flutter::DlBlendMode::kSaturation:
      return BlendMode::kSaturation;
    case flutter::DlBlendMode::kColor:
      return BlendMode::kColor;
    case flutter::DlBlendMode::kLuminosity:
      return BlendMode::kLuminosity;
  }
  FML_UNREACHABLE();
}

static Entity::TileMode ToTileMode(flutter::DlTileMode tile_mode) {
  switch (tile_mode) {
    case flutter::DlTileMode::kClamp:
      return Entity::TileMode::kClamp;
    case flutter::DlTileMode::kRepeat:
      return Entity::TileMode::kRepeat;
    case flutter::DlTileMode::kMirror:
      return Entity::TileMode::kMirror;
    case flutter::DlTileMode::kDecal:
      return Entity::TileMode::kDecal;
  }
}

static impeller::SamplerDescriptor ToSamplerDescriptor(
    const flutter::DlImageSampling options) {
  impeller::SamplerDescriptor desc;
  switch (options) {
    case flutter::DlImageSampling::kNearestNeighbor:
      desc.min_filter = desc.mag_filter = impeller::MinMagFilter::kNearest;
      desc.label = "Nearest Sampler";
      break;
    case flutter::DlImageSampling::kLinear:
    // Impeller doesn't support cubic sampling, but linear is closer to correct
    // than nearest for this case.
    case flutter::DlImageSampling::kCubic:
      desc.min_filter = desc.mag_filter = impeller::MinMagFilter::kLinear;
      desc.label = "Linear Sampler";
      break;
    case flutter::DlImageSampling::kMipmapLinear:
      desc.min_filter = desc.mag_filter = impeller::MinMagFilter::kLinear;
      desc.mip_filter = impeller::MipFilter::kLinear;
      desc.label = "Mipmap Linear Sampler";
      break;
  }
  return desc;
}

static Matrix ToMatrix(const SkMatrix& m) {
  return Matrix{
      // clang-format off
      m[0], m[3], 0, m[6],
      m[1], m[4], 0, m[7],
      0,    0,    1, 0,
      m[2], m[5], 0, m[8],
      // clang-format on
  };
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setAntiAlias(bool aa) {
  // Nothing to do because AA is implicit.
}

static Paint::Style ToStyle(flutter::DlDrawStyle style) {
  switch (style) {
    case flutter::DlDrawStyle::kFill:
      return Paint::Style::kFill;
    case flutter::DlDrawStyle::kStroke:
      return Paint::Style::kStroke;
    case flutter::DlDrawStyle::kStrokeAndFill:
      UNIMPLEMENTED;
      break;
  }
  return Paint::Style::kFill;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setDrawStyle(flutter::DlDrawStyle style) {
  paint_.style = ToStyle(style);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setColor(flutter::DlColor color) {
  paint_.color = {
      color.getRedF(),
      color.getGreenF(),
      color.getBlueF(),
      color.getAlphaF(),
  };
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setStrokeWidth(SkScalar width) {
  paint_.stroke_width = width;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setStrokeMiter(SkScalar limit) {
  paint_.stroke_miter = limit;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setStrokeCap(flutter::DlStrokeCap cap) {
  switch (cap) {
    case flutter::DlStrokeCap::kButt:
      paint_.stroke_cap = Cap::kButt;
      break;
    case flutter::DlStrokeCap::kRound:
      paint_.stroke_cap = Cap::kRound;
      break;
    case flutter::DlStrokeCap::kSquare:
      paint_.stroke_cap = Cap::kSquare;
      break;
  }
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setStrokeJoin(flutter::DlStrokeJoin join) {
  switch (join) {
    case flutter::DlStrokeJoin::kMiter:
      paint_.stroke_join = Join::kMiter;
      break;
    case flutter::DlStrokeJoin::kRound:
      paint_.stroke_join = Join::kRound;
      break;
    case flutter::DlStrokeJoin::kBevel:
      paint_.stroke_join = Join::kBevel;
      break;
  }
}

static std::optional<ColorSource::Type> ToColorSourceType(
    flutter::DlColorSourceType type) {
  switch (type) {
    case flutter::DlColorSourceType::kColor:
      return ColorSource::Type::kColor;
    case flutter::DlColorSourceType::kImage:
      return ColorSource::Type::kImage;
    case flutter::DlColorSourceType::kLinearGradient:
      return ColorSource::Type::kLinearGradient;
    case flutter::DlColorSourceType::kRadialGradient:
      return ColorSource::Type::kRadialGradient;
    case flutter::DlColorSourceType::kConicalGradient:
      return ColorSource::Type::kConicalGradient;
    case flutter::DlColorSourceType::kSweepGradient:
      return ColorSource::Type::kSweepGradient;
    case flutter::DlColorSourceType::kRuntimeEffect:
      return ColorSource::Type::kRuntimeEffect;
#ifdef IMPELLER_ENABLE_3D
    case flutter::DlColorSourceType::kScene:
      return ColorSource::Type::kScene;
#endif  // IMPELLER_ENABLE_3D
  }
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setColorSource(const flutter::DlColorSource* source) {
  if (!source) {
    paint_.color_source = ColorSource::MakeColor();
    return;
  }

  std::optional<ColorSource::Type> type = ToColorSourceType(source->type());

  if (!type.has_value()) {
    FML_LOG(ERROR) << "Requested ColorSourceType::kUnknown";
    paint_.color_source = ColorSource::MakeColor();
    return;
  }

  switch (type.value()) {
    case ColorSource::Type::kColor: {
      const flutter::DlColorColorSource* color = source->asColor();

      paint_.color_source = ColorSource::MakeColor();
      setColor(color->color());
      FML_DCHECK(color);
      return;
    }
    case ColorSource::Type::kLinearGradient: {
      const flutter::DlLinearGradientColorSource* linear =
          source->asLinearGradient();
      FML_DCHECK(linear);
      auto start_point = skia_conversions::ToPoint(linear->start_point());
      auto end_point = skia_conversions::ToPoint(linear->end_point());
      std::vector<Color> colors;
      std::vector<float> stops;
      skia_conversions::ConvertStops(linear, colors, stops);

      auto tile_mode = ToTileMode(linear->tile_mode());
      auto matrix = ToMatrix(linear->matrix());

      paint_.color_source = ColorSource::MakeLinearGradient(
          start_point, end_point, std::move(colors), std::move(stops),
          tile_mode, matrix);
      return;
    }
    case ColorSource::Type::kConicalGradient: {
      const flutter::DlConicalGradientColorSource* conical_gradient =
          source->asConicalGradient();
      FML_DCHECK(conical_gradient);
      Point center = skia_conversions::ToPoint(conical_gradient->end_center());
      SkScalar radius = conical_gradient->end_radius();
      Point focus_center =
          skia_conversions::ToPoint(conical_gradient->start_center());
      SkScalar focus_radius = conical_gradient->start_radius();
      std::vector<Color> colors;
      std::vector<float> stops;
      skia_conversions::ConvertStops(conical_gradient, colors, stops);

      auto tile_mode = ToTileMode(conical_gradient->tile_mode());
      auto matrix = ToMatrix(conical_gradient->matrix());

      paint_.color_source = ColorSource::MakeConicalGradient(
          center, radius, std::move(colors), std::move(stops), focus_center,
          focus_radius, tile_mode, matrix);
      return;
    }
    case ColorSource::Type::kRadialGradient: {
      const flutter::DlRadialGradientColorSource* radialGradient =
          source->asRadialGradient();
      FML_DCHECK(radialGradient);
      auto center = skia_conversions::ToPoint(radialGradient->center());
      auto radius = radialGradient->radius();
      std::vector<Color> colors;
      std::vector<float> stops;
      skia_conversions::ConvertStops(radialGradient, colors, stops);

      auto tile_mode = ToTileMode(radialGradient->tile_mode());
      auto matrix = ToMatrix(radialGradient->matrix());
      paint_.color_source =
          ColorSource::MakeRadialGradient(center, radius, std::move(colors),
                                          std::move(stops), tile_mode, matrix);
      return;
    }
    case ColorSource::Type::kSweepGradient: {
      const flutter::DlSweepGradientColorSource* sweepGradient =
          source->asSweepGradient();
      FML_DCHECK(sweepGradient);

      auto center = skia_conversions::ToPoint(sweepGradient->center());
      auto start_angle = Degrees(sweepGradient->start());
      auto end_angle = Degrees(sweepGradient->end());
      std::vector<Color> colors;
      std::vector<float> stops;
      skia_conversions::ConvertStops(sweepGradient, colors, stops);

      auto tile_mode = ToTileMode(sweepGradient->tile_mode());
      auto matrix = ToMatrix(sweepGradient->matrix());
      paint_.color_source = ColorSource::MakeSweepGradient(
          center, start_angle, end_angle, std::move(colors), std::move(stops),
          tile_mode, matrix);
      return;
    }
    case ColorSource::Type::kImage: {
      const flutter::DlImageColorSource* image_color_source = source->asImage();
      FML_DCHECK(image_color_source &&
                 image_color_source->image()->impeller_texture());
      auto texture = image_color_source->image()->impeller_texture();
      auto x_tile_mode = ToTileMode(image_color_source->horizontal_tile_mode());
      auto y_tile_mode = ToTileMode(image_color_source->vertical_tile_mode());
      auto desc = ToSamplerDescriptor(image_color_source->sampling());
      auto matrix = ToMatrix(image_color_source->matrix());
      paint_.color_source = ColorSource::MakeImage(texture, x_tile_mode,
                                                   y_tile_mode, desc, matrix);
      return;
    }
    case ColorSource::Type::kRuntimeEffect: {
      const flutter::DlRuntimeEffectColorSource* runtime_effect_color_source =
          source->asRuntimeEffect();
      auto runtime_stage =
          runtime_effect_color_source->runtime_effect()->runtime_stage();
      auto uniform_data = runtime_effect_color_source->uniform_data();
      auto samplers = runtime_effect_color_source->samplers();

      std::vector<RuntimeEffectContents::TextureInput> texture_inputs;

      for (auto& sampler : samplers) {
        if (sampler == nullptr) {
          return;
        }
        auto* image = sampler->asImage();
        if (!sampler->asImage()) {
          UNIMPLEMENTED;
          return;
        }
        FML_DCHECK(image->image()->impeller_texture());
        texture_inputs.push_back({
            .sampler_descriptor = ToSamplerDescriptor(image->sampling()),
            .texture = image->image()->impeller_texture(),
        });
      }

      paint_.color_source = ColorSource::MakeRuntimeEffect(
          runtime_stage, uniform_data, texture_inputs);
      return;
    }
    case ColorSource::Type::kScene: {
#ifdef IMPELLER_ENABLE_3D
      const flutter::DlSceneColorSource* scene_color_source = source->asScene();
      std::shared_ptr<scene::Node> scene_node =
          scene_color_source->scene_node();
      Matrix camera_transform = scene_color_source->camera_matrix();

      paint_.color_source =
          ColorSource::MakeScene(scene_node, camera_transform);
#else   // IMPELLER_ENABLE_3D
      FML_LOG(ERROR) << "ColorSourceType::kScene can only be used if Impeller "
                        "Scene is enabled.";
#endif  // IMPELLER_ENABLE_3D
      return;
    }
  }
}

static std::shared_ptr<ColorFilter> ToColorFilter(
    const flutter::DlColorFilter* filter) {
  if (filter == nullptr) {
    return nullptr;
  }
  switch (filter->type()) {
    case flutter::DlColorFilterType::kBlend: {
      auto dl_blend = filter->asBlend();
      auto blend_mode = ToBlendMode(dl_blend->mode());
      auto color = skia_conversions::ToColor(dl_blend->color());
      return ColorFilter::MakeBlend(blend_mode, color);
    }
    case flutter::DlColorFilterType::kMatrix: {
      const flutter::DlMatrixColorFilter* dl_matrix = filter->asMatrix();
      impeller::ColorMatrix color_matrix;
      dl_matrix->get_matrix(color_matrix.array);
      return ColorFilter::MakeMatrix(color_matrix);
    }
    case flutter::DlColorFilterType::kSrgbToLinearGamma:
      return ColorFilter::MakeSrgbToLinear();
    case flutter::DlColorFilterType::kLinearToSrgbGamma:
      return ColorFilter::MakeLinearToSrgb();
  }
  return nullptr;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setColorFilter(const flutter::DlColorFilter* filter) {
  paint_.color_filter = ToColorFilter(filter);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setInvertColors(bool invert) {
  paint_.invert_colors = invert;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setBlendMode(flutter::DlBlendMode dl_mode) {
  paint_.blend_mode = ToBlendMode(dl_mode);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setPathEffect(const flutter::DlPathEffect* effect) {
  // Needs https://github.com/flutter/flutter/issues/95434
  UNIMPLEMENTED;
}

static FilterContents::BlurStyle ToBlurStyle(flutter::DlBlurStyle blur_style) {
  switch (blur_style) {
    case flutter::DlBlurStyle::kNormal:
      return FilterContents::BlurStyle::kNormal;
    case flutter::DlBlurStyle::kSolid:
      return FilterContents::BlurStyle::kSolid;
    case flutter::DlBlurStyle::kOuter:
      return FilterContents::BlurStyle::kOuter;
    case flutter::DlBlurStyle::kInner:
      return FilterContents::BlurStyle::kInner;
  }
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setMaskFilter(const flutter::DlMaskFilter* filter) {
  // Needs https://github.com/flutter/flutter/issues/95434
  if (filter == nullptr) {
    paint_.mask_blur_descriptor = std::nullopt;
    return;
  }
  switch (filter->type()) {
    case flutter::DlMaskFilterType::kBlur: {
      auto blur = filter->asBlur();

      paint_.mask_blur_descriptor = {
          .style = ToBlurStyle(blur->style()),
          .sigma = Sigma(blur->sigma()),
      };
      break;
    }
  }
}

static std::shared_ptr<ImageFilter> ToImageFilter(
    const flutter::DlImageFilter* filter) {
  if (filter == nullptr) {
    return nullptr;
  }

  switch (filter->type()) {
    case flutter::DlImageFilterType::kBlur: {
      auto blur = filter->asBlur();
      auto sigma_x = Sigma(blur->sigma_x());
      auto sigma_y = Sigma(blur->sigma_y());
      auto tile_mode = ToTileMode(blur->tile_mode());
      return ImageFilter::MakeBlur(
          sigma_x, sigma_y, FilterContents::BlurStyle::kNormal, tile_mode);
    }
    case flutter::DlImageFilterType::kDilate: {
      auto dilate = filter->asDilate();
      FML_DCHECK(dilate);
      if (dilate->radius_x() < 0 || dilate->radius_y() < 0) {
        return nullptr;
      }
      auto radius_x = Radius(dilate->radius_x());
      auto radius_y = Radius(dilate->radius_y());
      return ImageFilter::MakeDilate(radius_x, radius_y);
    }
    case flutter::DlImageFilterType::kErode: {
      auto erode = filter->asErode();
      FML_DCHECK(erode);
      if (erode->radius_x() < 0 || erode->radius_y() < 0) {
        return nullptr;
      }
      auto radius_x = Radius(erode->radius_x());
      auto radius_y = Radius(erode->radius_y());
      return ImageFilter::MakeErode(radius_x, radius_y);
    }
    case flutter::DlImageFilterType::kMatrix: {
      auto matrix_filter = filter->asMatrix();
      FML_DCHECK(matrix_filter);
      auto matrix = ToMatrix(matrix_filter->matrix());
      auto desc = ToSamplerDescriptor(matrix_filter->sampling());
      return ImageFilter::MakeMatrix(matrix, desc);
    }
    case flutter::DlImageFilterType::kCompose: {
      auto compose = filter->asCompose();
      FML_DCHECK(compose);
      auto outer_dl_filter = compose->outer();
      auto inner_dl_filter = compose->inner();
      auto outer_filter = ToImageFilter(outer_dl_filter.get());
      auto inner_filter = ToImageFilter(inner_dl_filter.get());
      if (!outer_filter) {
        return inner_filter;
      }
      if (!inner_filter) {
        return outer_filter;
      }
      FML_DCHECK(outer_filter && inner_filter);

      return ImageFilter::MakeCompose(*inner_filter, *outer_filter);
    }
    case flutter::DlImageFilterType::kColorFilter: {
      auto color_filter_image_filter = filter->asColorFilter();
      FML_DCHECK(color_filter_image_filter);
      auto color_filter =
          ToColorFilter(color_filter_image_filter->color_filter().get());
      if (!color_filter) {
        return nullptr;
      }
      // When color filters are used as image filters, set the color filter's
      // "absorb opacity" flag to false. For image filters, the snapshot
      // opacity needs to be deferred until the result of the filter chain is
      // being blended with the layer.
      return ImageFilter::MakeFromColorFilter(*color_filter);
    }
    case flutter::DlImageFilterType::kLocalMatrix: {
      auto local_matrix_filter = filter->asLocalMatrix();
      FML_DCHECK(local_matrix_filter);
      auto internal_filter = local_matrix_filter->image_filter();
      FML_DCHECK(internal_filter);

      auto image_filter = ToImageFilter(internal_filter.get());
      if (!image_filter) {
        return nullptr;
      }

      auto matrix = ToMatrix(local_matrix_filter->matrix());
      return ImageFilter::MakeLocalMatrix(matrix, *image_filter);
    }
  }
}

// |flutter::DlOpReceiver|
void DlDispatcher2::setImageFilter(const flutter::DlImageFilter* filter) {
  paint_.image_filter = ToImageFilter(filter);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::save() {
  canvas_.Save();
}

// |flutter::DlOpReceiver|
void DlDispatcher2::saveLayer(const SkRect& bounds,
                              const flutter::SaveLayerOptions options,
                              const flutter::DlImageFilter* backdrop) {
  auto paint = options.renders_with_attributes() ? paint_ : Paint{};
  auto promise = options.content_is_clipped()
                     ? ContentBoundsPromise::kMayClipContents
                     : ContentBoundsPromise::kContainsContents;
  canvas_.SaveLayer(skia_conversions::ToRect(bounds), paint.blend_mode,
                    paint.color.alpha, promise);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::restore() {
  canvas_.Restore();
}

// |flutter::DlOpReceiver|
void DlDispatcher2::translate(SkScalar tx, SkScalar ty) {
  canvas_.Translate({tx, ty, 0.0});
}

// |flutter::DlOpReceiver|
void DlDispatcher2::scale(SkScalar sx, SkScalar sy) {
  canvas_.Scale({sx, sy, 1.0});
}

// |flutter::DlOpReceiver|
void DlDispatcher2::rotate(SkScalar degrees) {
  canvas_.Rotate(Degrees{degrees});
}

// |flutter::DlOpReceiver|
void DlDispatcher2::skew(SkScalar sx, SkScalar sy) {
  canvas_.Skew(sx, sy);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::transform2DAffine(SkScalar mxx,
                                      SkScalar mxy,
                                      SkScalar mxt,
                                      SkScalar myx,
                                      SkScalar myy,
                                      SkScalar myt) {
  // clang-format off
  transformFullPerspective(
    mxx, mxy,  0, mxt,
    myx, myy,  0, myt,
    0  ,   0,  1,   0,
    0  ,   0,  0,   1
  );
  // clang-format on
}

// |flutter::DlOpReceiver|
void DlDispatcher2::transformFullPerspective(SkScalar mxx,
                                             SkScalar mxy,
                                             SkScalar mxz,
                                             SkScalar mxt,
                                             SkScalar myx,
                                             SkScalar myy,
                                             SkScalar myz,
                                             SkScalar myt,
                                             SkScalar mzx,
                                             SkScalar mzy,
                                             SkScalar mzz,
                                             SkScalar mzt,
                                             SkScalar mwx,
                                             SkScalar mwy,
                                             SkScalar mwz,
                                             SkScalar mwt) {
  // The order of arguments is row-major but Impeller matrices are
  // column-major.
  // clang-format off
  auto transform = Matrix{
    mxx, myx, mzx, mwx,
    mxy, myy, mzy, mwy,
    mxz, myz, mzz, mwz,
    mxt, myt, mzt, mwt
  };
  // clang-format on
  canvas_.Transform(transform);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::transformReset() {
  canvas_.ResetTransform();
  canvas_.Transform(initial_matrix_);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::clipRect(const SkRect& rect, ClipOp clip_op, bool is_aa) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::clipRRect(const SkRRect& rrect, ClipOp sk_op, bool is_aa) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::clipPath(const SkPath& path, ClipOp sk_op, bool is_aa) {
  UNIMPLEMENTED;
}

const Path& DlDispatcher2::GetOrCachePath(const CacheablePath& cache) {
  if (cache.cached_impeller_path.IsEmpty() && !cache.sk_path.isEmpty()) {
    cache.cached_impeller_path = skia_conversions::ToPath(cache.sk_path);
  }
  return cache.cached_impeller_path;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::clipPath(const CacheablePath& cache,
                             ClipOp sk_op,
                             bool is_aa) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawColor(flutter::DlColor color,
                              flutter::DlBlendMode dl_mode) {
  auto ip_color = skia_conversions::ToColor(color);
  canvas_.DrawPaint(ip_color, ToBlendMode(dl_mode));
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawPaint() {
  canvas_.DrawPaint(paint_.color, paint_.blend_mode);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawLine(const SkPoint& p0, const SkPoint& p1) {
  canvas_.DrawLine(skia_conversions::ToPoint(p0), skia_conversions::ToPoint(p1),
                   paint_.color, paint_.blend_mode, paint_.stroke_width,
                   paint_.stroke_cap);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawRect(const SkRect& rect) {
  canvas_.DrawRect(skia_conversions::ToRect(rect), paint_.color,
                   paint_.blend_mode);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawOval(const SkRect& bounds) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawCircle(const SkPoint& center, SkScalar radius) {
  canvas_.DrawCircle(skia_conversions::ToPoint(center), radius, paint_.color,
                     paint_.blend_mode);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawRRect(const SkRRect& rrect) {
  if (rrect.isSimple()) {
    canvas_.DrawRRect(skia_conversions::ToRect(rrect.rect()),
                      skia_conversions::ToSize(rrect.getSimpleRadii()),
                      paint_.color, paint_.blend_mode);
  } else {
    canvas_.DrawPath(skia_conversions::ToPath(rrect), paint_.color,
                     paint_.blend_mode);
  }
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawDRRect(const SkRRect& outer, const SkRRect& inner) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawPath(const SkPath& path) {
  UNIMPLEMENTED;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawPath(const CacheablePath& cache) {
  canvas_.DrawPath(GetOrCachePath(cache), paint_.color, paint_.blend_mode);
}

void DlDispatcher2::SimplifyOrDrawPath(CanvasType& canvas,
                                       const CacheablePath& cache,
                                       const Paint& paint) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawArc(const SkRect& oval_bounds,
                            SkScalar start_degrees,
                            SkScalar sweep_degrees,
                            bool use_center) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawPoints(PointMode mode,
                               uint32_t count,
                               const SkPoint points[]) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawVertices(const flutter::DlVertices* vertices,
                                 flutter::DlBlendMode dl_mode) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawImage(const sk_sp<flutter::DlImage> image,
                              const SkPoint point,
                              flutter::DlImageSampling sampling,
                              bool render_with_attributes) {
  if (!image) {
    return;
  }

  auto texture = image->impeller_texture();
  if (!texture) {
    return;
  }

  const auto size = texture->GetSize();
  const auto src = SkRect::MakeWH(size.width, size.height);
  const auto dest =
      SkRect::MakeXYWH(point.fX, point.fY, size.width, size.height);

  drawImageRect(image,                      // image
                src,                        // source rect
                dest,                       // destination rect
                sampling,                   // sampling options
                render_with_attributes,     // render with attributes
                SrcRectConstraint::kStrict  // constraint
  );
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawImageRect(
    const sk_sp<flutter::DlImage> image,
    const SkRect& src,
    const SkRect& dst,
    flutter::DlImageSampling sampling,
    bool render_with_attributes,
    SrcRectConstraint constraint = SrcRectConstraint::kFast) {
  canvas_.DrawImageRect(image->impeller_texture(),      // image
                        skia_conversions::ToRect(src),  // source rect
                        skia_conversions::ToRect(dst),  // destination rect
                        render_with_attributes
                            ? paint_.blend_mode
                            : BlendMode::kSourceOver,   // paint
                        ToSamplerDescriptor(sampling),  // sampling
                        false);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawImageNine(const sk_sp<flutter::DlImage> image,
                                  const SkIRect& center,
                                  const SkRect& dst,
                                  flutter::DlFilterMode filter,
                                  bool render_with_attributes) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawAtlas(const sk_sp<flutter::DlImage> atlas,
                              const SkRSXform xform[],
                              const SkRect tex[],
                              const flutter::DlColor colors[],
                              int count,
                              flutter::DlBlendMode mode,
                              flutter::DlImageSampling sampling,
                              const SkRect* cull_rect,
                              bool render_with_attributes) {}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawDisplayList(
    const sk_sp<flutter::DisplayList> display_list,
    SkScalar opacity) {
  // Save all values that must remain untouched after the operation.
  Paint saved_paint = paint_;
  Matrix saved_initial_matrix = initial_matrix_;
  int restore_count = canvas_.GetSaveCount();

  // The display list may alter the clip, which must be restored to the current
  // clip at the end of playback.
  canvas_.Save();

  // Establish a new baseline for interpreting the new DL.
  // Matrix and clip are left untouched, the current
  // transform is saved as the new base matrix, and paint
  // values are reset to defaults.
  initial_matrix_ = canvas_.GetCurrentTransform();
  paint_ = Paint();

  // Handle passed opacity in the most brute-force way by using
  // a SaveLayer. If the display_list is able to inherit the
  // opacity, this could also be handled by modulating all of its
  // attribute settings (for example, color), by the indicated
  // opacity.
  // if (opacity < SK_Scalar1) {
  //   Paint save_paint;
  //   save_paint.color = Color(0, 0, 0, opacity);
  //   canvas_.SaveLayer(save_paint);
  // }

  // TODO(131445): Remove this restriction if we can correctly cull with
  // perspective transforms.
  if (display_list->has_rtree() && !initial_matrix_.HasPerspective()) {
    // The canvas remembers the screen-space culling bounds clipped by
    // the surface and the history of clip calls. DisplayList can cull
    // the ops based on a rectangle expressed in its "destination bounds"
    // so we need the canvas to transform those into the current local
    // coordinate space into which the DisplayList will be rendered.
    auto cull_bounds = canvas_.GetCurrentLocalCullingBounds();
    if (cull_bounds.has_value()) {
      Rect cull_rect = cull_bounds.value();
      display_list->Dispatch(
          *this, SkRect::MakeLTRB(cull_rect.GetLeft(), cull_rect.GetTop(),
                                  cull_rect.GetRight(), cull_rect.GetBottom()));
    } else {
      display_list->Dispatch(*this);
    }
  } else {
    display_list->Dispatch(*this);
  }

  // Restore all saved state back to what it was before we interpreted
  // the display_list
  canvas_.RestoreToCount(restore_count);
  initial_matrix_ = saved_initial_matrix;
  paint_ = saved_paint;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawTextBlob(const sk_sp<SkTextBlob> blob,
                                 SkScalar x,
                                 SkScalar y) {
  // When running with Impeller enabled Skia text blobs are converted to
  // Impeller text frames in paragraph_skia.cc
  UNIMPLEMENTED;
}

void DlDispatcher2::drawTextFrame(const std::shared_ptr<TextFrame>& text_frame,
                                  SkScalar x,
                                  SkScalar y) {
  canvas_.DrawTextFrame(text_frame, Point(x, y), paint_.color,
                        paint_.blend_mode);
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawShadow(const SkPath& path,
                               const flutter::DlColor color,
                               const SkScalar elevation,
                               bool transparent_occluder,
                               SkScalar dpr) {
  UNIMPLEMENTED;
}

// |flutter::DlOpReceiver|
void DlDispatcher2::drawShadow(const CacheablePath& cache,
                               const flutter::DlColor color,
                               const SkScalar elevation,
                               bool transparent_occluder,
                               SkScalar dpr) {}

//////////////////  GlyphAndCLipCollector ///////////////////

void GlyphAndCLipCollector::setAntiAlias(bool aa) {}

void GlyphAndCLipCollector::setDrawStyle(flutter::DlDrawStyle style) {}

void GlyphAndCLipCollector::setColor(flutter::DlColor color) {}

void GlyphAndCLipCollector::setStrokeWidth(SkScalar width) {}

void GlyphAndCLipCollector::setStrokeMiter(SkScalar limit) {}

void GlyphAndCLipCollector::setStrokeCap(flutter::DlStrokeCap cap) {}

void GlyphAndCLipCollector::setStrokeJoin(flutter::DlStrokeJoin join) {}

void GlyphAndCLipCollector::setColorSource(
    const flutter::DlColorSource* source) {}

void GlyphAndCLipCollector::setColorFilter(
    const flutter::DlColorFilter* filter) {}

void GlyphAndCLipCollector::setInvertColors(bool invert) {}

void GlyphAndCLipCollector::setBlendMode(flutter::DlBlendMode dl_mode) {}

void GlyphAndCLipCollector::setPathEffect(const flutter::DlPathEffect* effect) {
}

void GlyphAndCLipCollector::setMaskFilter(const flutter::DlMaskFilter* filter) {
}

void GlyphAndCLipCollector::setImageFilter(
    const flutter::DlImageFilter* filter) {}

void GlyphAndCLipCollector::save() {}

void GlyphAndCLipCollector::saveLayer(const SkRect& bounds,
                                      const flutter::SaveLayerOptions options,
                                      const flutter::DlImageFilter* backdrop) {}

void GlyphAndCLipCollector::restore() {}

void GlyphAndCLipCollector::translate(SkScalar tx, SkScalar ty) {}

void GlyphAndCLipCollector::scale(SkScalar sx, SkScalar sy) {}

void GlyphAndCLipCollector::rotate(SkScalar degrees) {}

void GlyphAndCLipCollector::skew(SkScalar sx, SkScalar sy) {}

void GlyphAndCLipCollector::transform2DAffine(SkScalar mxx,
                                              SkScalar mxy,
                                              SkScalar mxt,
                                              SkScalar myx,
                                              SkScalar myy,
                                              SkScalar myt) {}

void GlyphAndCLipCollector::transformFullPerspective(SkScalar mxx,
                                                     SkScalar mxy,
                                                     SkScalar mxz,
                                                     SkScalar mxt,
                                                     SkScalar myx,
                                                     SkScalar myy,
                                                     SkScalar myz,
                                                     SkScalar myt,
                                                     SkScalar mzx,
                                                     SkScalar mzy,
                                                     SkScalar mzz,
                                                     SkScalar mzt,
                                                     SkScalar mwx,
                                                     SkScalar mwy,
                                                     SkScalar mwz,
                                                     SkScalar mwt) {}

void GlyphAndCLipCollector::transformReset() {}

void GlyphAndCLipCollector::clipRect(const SkRect& rect,
                                     ClipOp clip_op,
                                     bool is_aa) {}

void GlyphAndCLipCollector::clipRRect(const SkRRect& rrect,
                                      ClipOp sk_op,
                                      bool is_aa) {}

void GlyphAndCLipCollector::clipPath(const SkPath& path,
                                     ClipOp sk_op,
                                     bool is_aa) {}

void GlyphAndCLipCollector::clipPath(const CacheablePath& cache,
                                     ClipOp sk_op,
                                     bool is_aa) {}

void GlyphAndCLipCollector::drawColor(flutter::DlColor color,
                                      flutter::DlBlendMode dl_mode) {}

void GlyphAndCLipCollector::drawPaint() {}

void GlyphAndCLipCollector::drawLine(const SkPoint& p0, const SkPoint& p1) {}

void GlyphAndCLipCollector::drawRect(const SkRect& rect) {}

void GlyphAndCLipCollector::drawOval(const SkRect& bounds) {}

void GlyphAndCLipCollector::drawCircle(const SkPoint& center, SkScalar radius) {
}

void GlyphAndCLipCollector::drawRRect(const SkRRect& rrect) {}

void GlyphAndCLipCollector::drawDRRect(const SkRRect& outer,
                                       const SkRRect& inner) {}

void GlyphAndCLipCollector::drawPath(const SkPath& path) {}

void GlyphAndCLipCollector::drawPath(const CacheablePath& cache) {}

void GlyphAndCLipCollector::drawArc(const SkRect& oval_bounds,
                                    SkScalar start_degrees,
                                    SkScalar sweep_degrees,
                                    bool use_center) {}

void GlyphAndCLipCollector::drawPoints(PointMode mode,
                                       uint32_t count,
                                       const SkPoint points[]) {}

void GlyphAndCLipCollector::drawVertices(const flutter::DlVertices* vertices,
                                         flutter::DlBlendMode dl_mode) {}

void GlyphAndCLipCollector::drawImage(const sk_sp<flutter::DlImage> image,
                                      const SkPoint point,
                                      flutter::DlImageSampling sampling,
                                      bool render_with_attributes) {}

void GlyphAndCLipCollector::drawImageRect(
    const sk_sp<flutter::DlImage> image,
    const SkRect& src,
    const SkRect& dst,
    flutter::DlImageSampling sampling,
    bool render_with_attributes,
    SrcRectConstraint constraint = SrcRectConstraint::kFast) {}

void GlyphAndCLipCollector::drawImageNine(const sk_sp<flutter::DlImage> image,
                                          const SkIRect& center,
                                          const SkRect& dst,
                                          flutter::DlFilterMode filter,
                                          bool render_with_attributes) {}

void GlyphAndCLipCollector::drawAtlas(const sk_sp<flutter::DlImage> atlas,
                                      const SkRSXform xform[],
                                      const SkRect tex[],
                                      const flutter::DlColor colors[],
                                      int count,
                                      flutter::DlBlendMode mode,
                                      flutter::DlImageSampling sampling,
                                      const SkRect* cull_rect,
                                      bool render_with_attributes) {}

void GlyphAndCLipCollector::drawDisplayList(
    const sk_sp<flutter::DisplayList> display_list,
    SkScalar opacity) {
  display_list->Dispatch(*this);
}

void GlyphAndCLipCollector::drawTextBlob(const sk_sp<SkTextBlob> blob,
                                         SkScalar x,
                                         SkScalar y) {}

void GlyphAndCLipCollector::drawTextFrame(
    const std::shared_ptr<TextFrame>& text_frame,
    SkScalar x,
    SkScalar y) {
  atlas_.AddTextFrame(*text_frame, 2.625);
}

void GlyphAndCLipCollector::drawShadow(const SkPath& path,
                                       const flutter::DlColor color,
                                       const SkScalar elevation,
                                       bool transparent_occluder,
                                       SkScalar dpr) {}

void GlyphAndCLipCollector::drawShadow(const CacheablePath& cache,
                                       const flutter::DlColor color,
                                       const SkScalar elevation,
                                       bool transparent_occluder,
                                       SkScalar dpr) {}

}  // namespace impeller
