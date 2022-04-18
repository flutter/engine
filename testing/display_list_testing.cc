// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/display_list_testing.h"

#include <iomanip>

#include "flutter/display_list/display_list.h"

namespace flutter {
namespace testing {

// clang-format off
bool DisplayListsEQ_Verbose(const DisplayList* a, const DisplayList* b) {
  if (a->Equals(b)) {
    return true;
  }
  FML_LOG(ERROR) << std::endl
                 << std::endl
                 << *a << std::endl
                 << "not identical to ..." << std::endl
                 << std::endl
                 << *b;
  return false;
}

bool DisplayListsNE_Verbose(const DisplayList* a, const DisplayList* b) {
  if (a->Equals(b)) {
    FML_LOG(ERROR) << std::endl
                   << "DisplayLists are both the same:" << std::endl
                   << *a;
    return false;
  }
  return true;
}

std::ostream& operator<<(std::ostream& os,
                         const DisplayList& display_list) {
  DisplayListStreamDispatcher dispatcher(os);
  os << std::boolalpha;
  os << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
  os << "DisplayList {" << std::endl;
  display_list.Dispatch(dispatcher);
  os << "}" << std::endl;
  return os;
}

std::ostream& operator<<(std::ostream& os, const DlBlendMode& mode) {
  switch (mode) {
    case DlBlendMode::kClear:      return os << "BlendMode::kClear";
    case DlBlendMode::kSrc:        return os << "BlendMode::kSrc";
    case DlBlendMode::kDst:        return os << "BlendMode::kDst";
    case DlBlendMode::kSrcOver:    return os << "BlendMode::kSrcOver";
    case DlBlendMode::kDstOver:    return os << "BlendMode::kDstOver";
    case DlBlendMode::kSrcIn:      return os << "BlendMode::kSrcIn";
    case DlBlendMode::kDstIn:      return os << "BlendMode::kDstIn";
    case DlBlendMode::kSrcOut:     return os << "BlendMode::kSrcOut";
    case DlBlendMode::kDstOut:     return os << "BlendMode::kDstOut";
    case DlBlendMode::kSrcATop:    return os << "BlendMode::kSrcATop";
    case DlBlendMode::kDstATop:    return os << "BlendMode::kDstATop";
    case DlBlendMode::kXor:        return os << "BlendMode::kXor";
    case DlBlendMode::kPlus:       return os << "BlendMode::kPlus";
    case DlBlendMode::kModulate:   return os << "BlendMode::kModulate";
    case DlBlendMode::kScreen:     return os << "BlendMode::kScreen";

    case DlBlendMode::kOverlay:    return os << "BlendMode::kOverlay";
    case DlBlendMode::kDarken:     return os << "BlendMode::kDarken";
    case DlBlendMode::kLighten:    return os << "BlendMode::kLighten";
    case DlBlendMode::kColorDodge: return os << "BlendMode::kColorDodge";
    case DlBlendMode::kColorBurn:  return os << "BlendMode::kColorBurn";
    case DlBlendMode::kHardLight:  return os << "BlendMode::kHardLight";
    case DlBlendMode::kSoftLight:  return os << "BlendMode::kSoftLight";
    case DlBlendMode::kDifference: return os << "BlendMode::kDifference";
    case DlBlendMode::kExclusion:  return os << "BlendMode::kExclusion";
    case DlBlendMode::kMultiply:   return os << "BlendMode::kMultiply";

    case DlBlendMode::kHue:        return os << "BlendMode::kHue";
    case DlBlendMode::kSaturation: return os << "BlendMode::kSaturation";
    case DlBlendMode::kColor:      return os << "BlendMode::kColor";
    case DlBlendMode::kLuminosity: return os << "BlendMode::kLuminosity";

    default: return os << "BlendMode::????";
  }
}

std::ostream& operator<<(std::ostream& os, const SaveLayerOptions& options) {
  return os << "SaveLayerOptions("
            << "can_distribute_opacity: " << options.can_distribute_opacity()
            << ", "
            << "renders_with_attributes: " << options.renders_with_attributes()
            << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkPoint& point) {
  return os << "SkPoint(" << point.fX << ", " << point.fY << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkIRect& rect) {
  return os << "SkIRect("
            << "left: " << rect.fLeft << ", "
            << "top: " << rect.fTop << ", "
            << "right: " << rect.fRight << ", "
            << "bottom: " << rect.fBottom
            << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkRect& rect) {
  return os << "SkRect("
            << "left: " << rect.fLeft << ", "
            << "top: " << rect.fTop << ", "
            << "right: " << rect.fRight << ", "
            << "bottom: " << rect.fBottom
            << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkRect* rect) {
  return rect ? os << *rect : os << "no rect";
}

static std::ostream& operator<<(std::ostream& os, const SkRRect& rrect) {
  return os << "SkRRect("
            << rrect.rect() << ", "
            << "ul: (" << rrect.radii(SkRRect::kUpperLeft_Corner).fX << ", "
                       << rrect.radii(SkRRect::kUpperLeft_Corner).fY << "), "
            << "ur: (" << rrect.radii(SkRRect::kUpperRight_Corner).fX << ", "
                       << rrect.radii(SkRRect::kUpperRight_Corner).fY << "), "
            << "lr: (" << rrect.radii(SkRRect::kLowerRight_Corner).fX << ", "
                       << rrect.radii(SkRRect::kLowerRight_Corner).fY << "), "
            << "ll: (" << rrect.radii(SkRRect::kLowerLeft_Corner).fX << ", "
                       << rrect.radii(SkRRect::kLowerLeft_Corner).fY << ")"
            << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkPath& path) {
  return os << "SkPath("
            << "bounds: " << path.getBounds()
            // should iterate over verbs and coordinates...
            << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkMatrix& matrix) {
  return os << "SkMatrix("
            << "[" << matrix[0] << ", " << matrix[1] << ", " << matrix[2] << "], "
            << "[" << matrix[3] << ", " << matrix[4] << ", " << matrix[5] << "], "
            << "[" << matrix[6] << ", " << matrix[7] << ", " << matrix[8] << "]"
            << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkMatrix* matrix) {
  if (matrix) return os << *matrix;
  return os << "no matrix";
}

static std::ostream& operator<<(std::ostream& os, const SkRSXform& xform) {
  return os << "SkRSXform("
            << "scos: " << xform.fSCos << ", "
            << "ssin: " << xform.fSSin << ", "
            << "tx: " << xform.fTx << ", "
            << "ty: " << xform.fTy << ")";
}

static std::ostream& operator<<(std::ostream& os, const SkClipOp& op) {
  switch (op) {
    case SkClipOp::kDifference: return os << "ClipOp::kDifference";
    case SkClipOp::kIntersect:  return os << "ClipOp::kIntersect";

    default: return os << "ClipOp::????";
  }
}

static std::ostream& operator<<(std::ostream& os, const SkPaint::Cap& cap) {
  switch (cap) {
    case SkPaint::kButt_Cap:   return os << "Cap::kButt";
    case SkPaint::kRound_Cap:  return os << "Cap::kRound";
    case SkPaint::kSquare_Cap: return os << "Cap::kSquare";

    default: return os << "Cap::????";
  }
}

static std::ostream& operator<<(std::ostream& os, const SkPaint::Join& join) {
  switch (join) {
    case SkPaint::kMiter_Join: return os << "Join::kMiter";
    case SkPaint::kRound_Join: return os << "Join::kRound";
    case SkPaint::kBevel_Join: return os << "Join::kBevel";

    default: return os << "Join::????";
  }
}

static std::ostream& operator<<(std::ostream& os, const SkPaint::Style& style) {
  switch (style) {
    case SkPaint::kFill_Style:          return os << "Style::kFill";
    case SkPaint::kStroke_Style:        return os << "Style::kStroke";
    case SkPaint::kStrokeAndFill_Style: return os << "Style::kStrokeAnFill";

    default: return os << "Style::????";
  }
}

static std::ostream& operator<<(std::ostream& os,
                                const SkCanvas::PointMode& mode) {
  switch (mode) {
    case SkCanvas::kPoints_PointMode:  return os << "PointMode::kPoints";
    case SkCanvas::kLines_PointMode:   return os << "PointMode::kLines";
    case SkCanvas::kPolygon_PointMode: return os << "PointMode::kPolygon";

    default: return os << "PointMode::????";
  }
}

static std::ostream& operator<<(std::ostream& os, const SkFilterMode& mode) {
  switch (mode) {
    case SkFilterMode::kNearest: return os << "FilterMode::kNearest";
    case SkFilterMode::kLinear:  return os << "FilterMode::kLinear";

    default: return os << "FilterMode::????";
  }
}

class SK_Color {
 public:
  explicit SK_Color(SkColor color) : color(color) {}
  SkColor color;
};

static std::ostream& operator<<(std::ostream& os, const SK_Color& color) {
  return os << "SkColor(" << std::hex << color.color << std::dec << ")";
}

static std::ostream& operator<<(std::ostream& os,
                                const SkSamplingOptions sampling) {
  if (sampling == DisplayList::NearestSampling) {
    return os << "NearestSampling";
  }
  if (sampling == DisplayList::LinearSampling) {
    return os << "LinearSampling";
  }
  if (sampling == DisplayList::MipmapSampling) {
    return os << "MipmapSampling";
  }
  if (sampling == DisplayList::CubicSampling) {
    return os << "CubicSampling";
  }
  return os << "UnkownSampling";
}

static std::ostream& operator<<(std::ostream& os, const SkTextBlob* blob) {
  if (blob == nullptr) {
    return os << "no text";
  }
  return os << "SkTextBlob(ID: " << blob->uniqueID() << ", " << blob->bounds() << ")";
}

static std::ostream& operator<<(std::ostream& os, const DlVertexMode& mode) {
  switch (mode) {
    case DlVertexMode::kTriangles:     return os << "VertexMode::kTriangles";
    case DlVertexMode::kTriangleStrip: return os << "VertexMode::kTriangleStrip";
    case DlVertexMode::kTriangleFan:   return os << "VertexMode::kTriangleFan";

    default: return os << "VertexMode::????";
  }
}

static std::ostream& operator<<(std::ostream& os, const DlTileMode& mode) {
  switch (mode) {
    case DlTileMode::kClamp: return os << "TileMode::kClamp";
    case DlTileMode::kRepeat: return os << "TileMode::kRepeat";
    case DlTileMode::kMirror: return os << "TileMode::kMirror";
    case DlTileMode::kDecal: return os << "TileMode::kDecal";

    default: return os << "TileMode::????";
  }
}

static std::ostream& operator<<(std::ostream& os, const DlImage* image) {
  if (image == nullptr) {
    return os << "null image";
  }
  os << "DlImage(" << image->width() << " x " << image->height() << ", ";
  if (image->skia_image()) {
    os << "skia(" << image->skia_image().get() << "), ";
  }
  if (image->impeller_texture()) {
    os << "impeller(" << image->impeller_texture().get() << "), ";
  }
  return os << "isTextureBacked: " << image->isTextureBacked() << ")";
}

std::ostream& DisplayListStreamDispatcher::startl() {
  for (int i = 0; i < cur_indent_; i++) {
    os_ << " ";
  }
  return os_;
}

std::ostream& DisplayListStreamDispatcher::out_points(std::string name,
                                                      int count,
                                                      const SkPoint points[]) {
  if (points == nullptr || count < 0) {
    return os_ << "no " << name;
  }
  os_ << name << "[" << count << "] = [" << std::endl;
  indent();
  indent();
  for (int i = 0; i < count; i++) {
    startl() << points[i] << "," << std::endl;
  }
  outdent();
  startl() << "]";
  outdent();
  return os_;
}

std::ostream& DisplayListStreamDispatcher::out_rects(std::string name,
                                                     int count,
                                                     const SkRect rects[]) {
  if (rects == nullptr || count < 0) {
    return os_ << "no " << name;
  }
  os_ << name << "[" << count << "] = [" << std::endl;
  indent();
  indent();
  for (int i = 0; i < count; i++) {
    startl() << rects[i] << "," << std::endl;
  }
  outdent();
  startl() << "]";
  outdent();
  return os_;
}

std::ostream& DisplayListStreamDispatcher::out_colors(std::string name,
                                                      int count,
                                                      const SkColor colors[]) {
  if (colors == nullptr || count < 0) {
    return os_ << "no " << name;
  }
  os_ << name << "[" << count << "] = [" << std::endl;
  indent();
  indent();
  for (int i = 0; i < count; i++) {
    startl() << SK_Color(colors[i]) << "," << std::endl;
  }
  outdent();
  startl() << "]";
  outdent();
  return os_;
}

std::ostream& DisplayListStreamDispatcher::out_indices(std::string name,
                                                       int count,
                                                       const uint16_t indices[]) {
  if (indices == nullptr || count < 0) {
    return os_ << "no " << name;
  }
  os_ << name << "[" << count << "] = [" << std::endl;
  indent();
  indent();
  for (int i = 0; i < count; i++) {
    startl() << indices[i] << "," << std::endl;
  }
  outdent();
  startl() << "]";
  outdent();
  return os_;
}

std::ostream& DisplayListStreamDispatcher::out_floats(std::string name,
                                                      int count,
                                                      const float floats[]) {
  if (floats == nullptr || count < 0) {
    return os_ << "no " << name;
  }
  os_ << name << "[" << count << "] = [" << std::endl;
  indent();
  indent();
  for (int i = 0; i < count; i++) {
    startl() << floats[i] << "," << std::endl;
  }
  outdent();
  startl() << "]";
  outdent();
  return os_;
}

std::ostream& DisplayListStreamDispatcher::out_xforms(std::string name,
                                                      int count,
                                                      const SkRSXform xforms[]) {
  if (xforms == nullptr || count < 0) {
    return os_ << "no " << name;
  }
  os_ << name << "[" << count << "] = [" << std::endl;
  indent();
  indent();
  for (int i = 0; i < count; i++) {
    startl() << xforms[i] << "," << std::endl;
  }
  outdent();
  startl() << "]";
  outdent();
  return os_;
}

void DisplayListStreamDispatcher::setAntiAlias(bool aa) {
  startl() << "setAntiAlias(" << aa << ");" << std::endl;
}
void DisplayListStreamDispatcher::setDither(bool dither) {
  startl() << "setDither(" << dither << ");" << std::endl;
}
void DisplayListStreamDispatcher::setStyle(SkPaint::Style style) {
  startl() << "setStyle(" << style << ");" << std::endl;
}
void DisplayListStreamDispatcher::setColor(SkColor color) {
  startl() << "setColor(" << SK_Color(color) << ");" << std::endl;
}
void DisplayListStreamDispatcher::setStrokeWidth(SkScalar width) {
  startl() << "setStrokeWidth(" << width << ");" << std::endl;
}
void DisplayListStreamDispatcher::setStrokeMiter(SkScalar limit) {
  startl() << "setStrokeMiter(" << limit << ");" << std::endl;
}
void DisplayListStreamDispatcher::setStrokeCap(SkPaint::Cap cap) {
  startl() << "setStrokeCap(" << cap << ");" << std::endl;
}
void DisplayListStreamDispatcher::setStrokeJoin(SkPaint::Join join) {
  startl() << "setStrokeJoin(" << join << ");" << std::endl;
}
void DisplayListStreamDispatcher::setColorSource(const DlColorSource* source) {
  if (source == nullptr) {
    startl() << "setColorSource(no ColorSource);" << std::endl;
    return;
  }
  startl() << "setColorSource(";
  switch (source->type()) {
    case DlColorSourceType::kColor: {
      const DlColorColorSource* color_src = source->asColor();
      os_ << "DlColorColorSource(" << color_src->color() << ")";
      break;
    }
    case DlColorSourceType::kImage: {
      const DlImageColorSource* image_src = source->asImage();
      os_ << "DlImageColorSource(" << image_src->image()
                           << ", hMode: " << image_src->horizontal_tile_mode()
                           << ", vMode: " << image_src->vertical_tile_mode()
                           << ", " << image_src->sampling()
                           << ", " << image_src->matrix_ptr()
                           << ")";
      break;
    }
    case DlColorSourceType::kLinearGradient: {
      const DlLinearGradientColorSource* linear_src = source->asLinearGradient();
      os_ << "DlLinearGradientSource("
                                 << "start: " << linear_src->start_point()
                                 << ", end: " << linear_src->end_point() << ", ";
                                 out_colors("colors", linear_src->stop_count(), linear_src->colors()) << ", ";
                                 out_floats("stops", linear_src->stop_count(), linear_src->stops()) << ", "
                                 << linear_src->tile_mode() << ", " << linear_src->matrix_ptr() << ")";
      break;
    }
    case DlColorSourceType::kRadialGradient: {
      const DlRadialGradientColorSource* radial_src = source->asRadialGradient();
      os_ << "DlRadialGradientSource("
                                 << "center: " << radial_src->center()
                                 << ", radius: " << radial_src->radius() << ", ";
                                 out_colors("colors", radial_src->stop_count(), radial_src->colors()) << ", ";
                                 out_floats("stops", radial_src->stop_count(), radial_src->stops()) << ", "
                                 << radial_src->tile_mode() << ", " << radial_src->matrix_ptr() << ")";
      break;
    }
    case DlColorSourceType::kSweepGradient: {
      const DlSweepGradientColorSource* radial_src = source->asSweepGradient();
      os_ << "DlSweepGradientColorSource("
                                 << "center: " << radial_src->center()
                                 << ", start: " << radial_src->start() << ", "
                                 << ", end: " << radial_src->end() << ", ";
                                 out_colors("colors", radial_src->stop_count(), radial_src->colors()) << ", ";
                                 out_floats("stops", radial_src->stop_count(), radial_src->stops()) << ", "
                                 << radial_src->tile_mode() << ", " << radial_src->matrix_ptr() << ")";
      break;
    }
    case DlColorSourceType::kConicalGradient: {
      const DlConicalGradientColorSource* radial_src = source->asConicalGradient();
      os_ << "DlConicalGradientColorSource("
                                 << "start center: " << radial_src->start_center()
                                 << ", start radius: " << radial_src->start_radius()
                                 << ", end center: " << radial_src->end_center()
                                 << ", end radius: " << radial_src->end_radius() << ", ";
                                 out_colors("colors", radial_src->stop_count(), radial_src->colors()) << ", ";
                                 out_floats("stops", radial_src->stop_count(), radial_src->stops()) << ", "
                                 << radial_src->tile_mode() << ", " << radial_src->matrix_ptr() << ")";
      break;
    }
    default:
      os_ << "DlUnknownColorSource(" << source->skia_object().get() << ")";
      break;
  }
  os_ << ");" << std::endl;
}
void DisplayListStreamDispatcher::setColorFilter(const DlColorFilter* filter) {
  if (filter == nullptr) {
    startl() << "setColorFilter(no ColorFilter);" << std::endl;
    return;
  }
  startl() << "setColorFilter(";
  switch (filter->type()) {
    case DlColorFilterType::kBlend: {
      const DlBlendColorFilter* blend = filter->asBlend();
      os_ << "DlBlendColorFilter(" << SK_Color(blend->color()) << ", "
                                   << static_cast<int>(blend->mode()) << ")";
      break;
    }
    case DlColorFilterType::kMatrix: {
      const DlMatrixColorFilter* matrix = filter->asMatrix();
      float values[20];
      matrix->get_matrix(values);
      os_ << "DlMatrixColorFilter(matrix[20] = [" << std::endl;
      indent();
      for (int i = 0; i < 20; i += 5) {
        startl() << values[i] << ", "
                 << values[i+1] << ", "
                 << values[i+2] << ", "
                 << values[i+3] << ", "
                 << values[i+4] << ","
                 << std::endl;
      }
      startl() << "]";
      outdent();
      break;
    }
    case DlColorFilterType::kLinearToSrgbGamma: {
      os_ << "DlLinearToSrgbGammaColorFilter()";
      break;
    }
    case DlColorFilterType::kSrgbToLinearGamma: {
      os_ << "DlSrgbToLinearGammaColorFilter()";
      break;
    }
    default:
      os_ << "DlUnknownColorFilter(" << filter->skia_object().get() << ")";
      break;
  }
  os_ << ");" << std::endl;
}
void DisplayListStreamDispatcher::setInvertColors(bool invert) {
  startl() << "setInvertColors(" << invert << ");" << std::endl;
}
void DisplayListStreamDispatcher::setBlendMode(DlBlendMode mode) {
  startl() << "setBlendMode(" << mode << ");" << std::endl;
}
void DisplayListStreamDispatcher::setBlender(sk_sp<SkBlender> blender) {
  startl() << "setBlender(" << blender << ");" << std::endl;
}
void DisplayListStreamDispatcher::setPathEffect(sk_sp<SkPathEffect> effect) {
  startl() << "setPathEffect(" << effect << ");" << std::endl;
}
void DisplayListStreamDispatcher::setMaskFilter(const DlMaskFilter* filter) {
  startl() << "setMaskFilter(" << filter << ");" << std::endl;
}
void DisplayListStreamDispatcher::setImageFilter(const DlImageFilter* filter) {
  startl() << "setImageFilter(" << filter << ");" << std::endl;
}

void DisplayListStreamDispatcher::save() {
  startl() << "save();" << std::endl;
  startl() << "{" << std::endl;
  indent();
}
void DisplayListStreamDispatcher::saveLayer(const SkRect* bounds,
                                            const SaveLayerOptions options) {
  startl() << "saveLayer(" << bounds << ", " << options << ");" << std::endl;
  startl() << "{" << std::endl;
  indent();
}
void DisplayListStreamDispatcher::restore() {
  startl() << "restore();" << std::endl;
  outdent();
  startl() << "}" << std::endl;
}

void DisplayListStreamDispatcher::translate(SkScalar tx, SkScalar ty) {
  startl() << "translate(" << tx << ", " << ty << ");" << std::endl;
}
void DisplayListStreamDispatcher::scale(SkScalar sx, SkScalar sy) {
  startl() << "scale(" << sx << ", " << sy << ");" << std::endl;
}
void DisplayListStreamDispatcher::rotate(SkScalar degrees) {
  startl() << "rotate(" << degrees << ");" << std::endl;
}
void DisplayListStreamDispatcher::skew(SkScalar sx, SkScalar sy) {
  startl() << "skew(" << sx << ", " << sy << ");" << std::endl;
}
void DisplayListStreamDispatcher::transform2DAffine(
    SkScalar mxx, SkScalar mxy, SkScalar mxt,
    SkScalar myx, SkScalar myy, SkScalar myt) {
  startl() << "transform2DAffine(" << std::endl;
  indent();
  {
    startl()
        << "[" << mxx << ", " << mxy << ", " << mxt << "], "
        << std::endl;
    startl()
        << "[" << myx << ", " << myy  << ", " << myt << "], "
        << std::endl;
  }
  outdent();
  startl() << ");" << std::endl;
}
void DisplayListStreamDispatcher::transformFullPerspective(
    SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
    SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
    SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
    SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) {
  startl() << "transformFullPerspective(" << std::endl;
  indent();
  {
    startl()
        << "[" << mxx << ", " << mxy << ", " << mxz << ", " << mxt << "], "
        << std::endl;
    startl()
        << "[" << myx << ", " << myy << ", " << myz << ", " << myt << "], "
        << std::endl;
    startl()
        << "[" << mzx << ", " << mzy << ", " << mzz << ", " << mzt << "], "
        << std::endl;
    startl()
        << "[" << mwx << ", " << mwy << ", " << mwz << ", " << mwt << "]"
        << std::endl;
  }
  outdent();
  startl() << ");" << std::endl;
}
void DisplayListStreamDispatcher::transformReset() {
  startl() << "tranformReset();" << std::endl;
}

void DisplayListStreamDispatcher::clipRect(const SkRect& rect, SkClipOp clip_op,
                                           bool is_aa) {
  startl() << "clipRect("
           << rect << ", "
           << clip_op << ", "
           << "isaa: " << is_aa
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::clipRRect(const SkRRect& rrect,
                         SkClipOp clip_op,
                         bool is_aa) {
  startl() << "clipRRect("
           << rrect << ", "
           << clip_op << ", "
           << "isaa: " << is_aa
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::clipPath(const SkPath& path, SkClipOp clip_op,
                                           bool is_aa) {
  startl() << "clipPath("
           << path << ", "
           << clip_op << ", "
           << "isaa: " << is_aa
           << ");" << std::endl;
}

void DisplayListStreamDispatcher::drawColor(SkColor color, DlBlendMode mode) {
  startl() << "drawColor("
           << SK_Color(color) << ", "
           << mode
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawPaint() {
  startl() << "drawPaint();" << std::endl;
}
void DisplayListStreamDispatcher::drawLine(const SkPoint& p0,
                                           const SkPoint& p1) {
  startl() << "drawLine(" << p0 << ", " << p1 << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawRect(const SkRect& rect) {
  startl() << "drawRect(" << rect << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawOval(const SkRect& bounds) {
  startl() << "drawOval(" << bounds << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawCircle(const SkPoint& center,
                                             SkScalar radius) {
  startl() << "drawCircle(" << center << ", " << radius << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawRRect(const SkRRect& rrect) {
  startl() << "drawRRect(" << rrect << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawDRRect(const SkRRect& outer,
                                             const SkRRect& inner) {
  startl() << "drawDRRect(outer: " << outer << ", " << std::endl;
  startl() << "           inner: " << inner << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawPath(const SkPath& path) {
  startl() << "drawPath(" << path << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawArc(const SkRect& oval_bounds,
                                          SkScalar start_degrees,
                                          SkScalar sweep_degrees,
                                          bool use_center) {
  startl() << "drawArc("
           << oval_bounds << ", "
           << start_degrees << ", "
           << sweep_degrees << ", "
           << "use_center: " << use_center
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawPoints(SkCanvas::PointMode mode,
                                             uint32_t count,
                                             const SkPoint points[]) {
  startl() << "drawPoints(" << mode << ", ";
                          out_points("points", count, points)
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawSkVertices(const sk_sp<SkVertices> vertices,
                                                 SkBlendMode mode) {
  startl() << "drawSkVertices(" << vertices << ", " << static_cast<int>(mode) << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawVertices(const DlVertices* vertices,
                                               DlBlendMode mode) {
  startl() << "drawVertices("
               << "DlVertices("
                   << vertices->mode() << ", ";
                   out_points("vertices", vertices->vertex_count(), vertices->vertices()) << ", ";
                   out_points("texture_coords", vertices->vertex_count(), vertices->texture_coordinates()) << ", ";
                   out_colors("colors", vertices->vertex_count(), vertices->colors()) << ", ";
                   out_indices("indices", vertices->index_count(), vertices->indices())
                   << "), " << mode << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawImage(const sk_sp<DlImage> image,
                                            const SkPoint point,
                                            const SkSamplingOptions& sampling,
                                            bool render_with_attributes) {
  startl() << "drawImage(" << image.get() << "," << std::endl;
  startl() << "          " << point << ", "
                           << sampling << ", "
                           << "with attributes: " << render_with_attributes
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawImageRect(const sk_sp<DlImage> image,
                                                const SkRect& src,
                                                const SkRect& dst,
                                                const SkSamplingOptions& sampling,
                                                bool render_with_attributes,
                                                SkCanvas::SrcRectConstraint constraint) {
  startl() << "drawImageRect(" << image.get() << "," << std::endl;
  startl() << "              src: " << src << "," << std::endl;
  startl() << "              dst: " << dst << "," << std::endl;
  startl() << "              " << sampling << ", "
                               << "with attributes: " << render_with_attributes << ", "
                               << constraint
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawImageNine(const sk_sp<DlImage> image,
                                                const SkIRect& center,
                                                const SkRect& dst,
                                                SkFilterMode filter,
                                                bool render_with_attributes) {
  startl() << "drawImageNine(" << image.get() << "," << std::endl;
  startl() << "              center: " << center << "," << std::endl;
  startl() << "              dst: " << dst << "," << std::endl;
  startl() << "              " << filter << ", "
                               << "with attributes: " << render_with_attributes
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawImageLattice(const sk_sp<DlImage> image,
                                                   const SkCanvas::Lattice& lattice,
                                                   const SkRect& dst,
                                                   SkFilterMode filter,
                                                   bool render_with_attributes) {
  startl() << "drawImageLattice(blah blah);" << std::endl;
}
void DisplayListStreamDispatcher::drawAtlas(const sk_sp<DlImage> atlas,
                                            const SkRSXform xform[],
                                            const SkRect tex[],
                                            const SkColor colors[],
                                            int count,
                                            DlBlendMode mode,
                                            const SkSamplingOptions& sampling,
                                            const SkRect* cull_rect,
                                            bool render_with_attributes) {
  startl() << "drawImageNine(" << atlas.get() << ", ";
                   out_xforms("xforms", count, xform) << ", ";
                   out_rects("tex_coords", count, tex) << ", ";
                   out_colors("colors", count, colors) << ", "
                   << mode << ", " << sampling << ", cull: " << cull_rect << ", "
                   << "with attributes: " << render_with_attributes
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawPicture(const sk_sp<SkPicture> picture,
                                              const SkMatrix* matrix,
                                              bool render_with_attributes) {
  startl() << "drawPicture("
           << "SkPicture(ID: " << picture->uniqueID() << ", bounds: " << picture->cullRect() << ", @" << picture << "), "
           << matrix << ", "
           << render_with_attributes
           << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawDisplayList(
    const sk_sp<DisplayList> display_list) {
  startl() << "drawDisplayList(ID: " << display_list->unique_id() << ", bounds: " << display_list->bounds() << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawTextBlob(const sk_sp<SkTextBlob> blob,
                                               SkScalar x,
                                               SkScalar y) {
  startl() << "drawTextBlob("
           << blob.get() << ", "
           << x << ", " << y << ");" << std::endl;
}
void DisplayListStreamDispatcher::drawShadow(const SkPath& path,
                                             const SkColor color,
                                             const SkScalar elevation,
                                             bool transparent_occluder,
                                             SkScalar dpr) {
  startl() << "drawShadow("
           << path << ", "
           << SK_Color(color) << ", "
           << elevation << ", "
           << transparent_occluder << ", "
           << dpr
           << ");" << std::endl;
}
// clang-format on

}  // namespace testing
}  // namespace flutter
