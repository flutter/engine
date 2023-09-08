// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_DISPLAY_LIST_TESTING_H_
#define TESTING_DISPLAY_LIST_TESTING_H_

#include <ostream>

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/dl_op_receiver.h"

namespace flutter {
namespace testing {

bool DisplayListsEQ_Verbose(const DisplayList* a, const DisplayList* b);
bool inline DisplayListsEQ_Verbose(const DisplayList& a, const DisplayList& b) {
  return DisplayListsEQ_Verbose(&a, &b);
}
bool inline DisplayListsEQ_Verbose(sk_sp<const DisplayList> a,
                                   sk_sp<const DisplayList> b) {
  return DisplayListsEQ_Verbose(a.get(), b.get());
}
bool DisplayListsNE_Verbose(const DisplayList* a, const DisplayList* b);
bool inline DisplayListsNE_Verbose(const DisplayList& a, const DisplayList& b) {
  return DisplayListsNE_Verbose(&a, &b);
}
bool inline DisplayListsNE_Verbose(sk_sp<const DisplayList> a,
                                   sk_sp<const DisplayList> b) {
  return DisplayListsNE_Verbose(a.get(), b.get());
}

extern std::ostream& operator<<(std::ostream& os,
                                const DisplayList& display_list);
extern std::ostream& operator<<(std::ostream& os, const DlPaint& paint);
extern std::ostream& operator<<(std::ostream& os, const DlBlendMode& mode);
extern std::ostream& operator<<(std::ostream& os, const DlCanvas::ClipOp& op);
extern std::ostream& operator<<(std::ostream& os,
                                const DlCanvas::PointMode& op);
extern std::ostream& operator<<(std::ostream& os,
                                const DlCanvas::SrcRectConstraint& op);
extern std::ostream& operator<<(std::ostream& os, const DlStrokeCap& cap);
extern std::ostream& operator<<(std::ostream& os, const DlStrokeJoin& join);
extern std::ostream& operator<<(std::ostream& os, const DlDrawStyle& style);
extern std::ostream& operator<<(std::ostream& os, const DlBlurStyle& style);
extern std::ostream& operator<<(std::ostream& os, const DlFilterMode& mode);
extern std::ostream& operator<<(std::ostream& os, const DlColor& color);
extern std::ostream& operator<<(std::ostream& os, DlImageSampling sampling);
extern std::ostream& operator<<(std::ostream& os, const DlVertexMode& mode);
extern std::ostream& operator<<(std::ostream& os, const DlTileMode& mode);
extern std::ostream& operator<<(std::ostream& os, const DlImage* image);

class DisplayListStreamDispatcher final : public DlOpReceiver {
 public:
  DisplayListStreamDispatcher(std::ostream& os,
                              int cur_indent = 2,
                              int indent = 2)
      : os_(os), cur_indent_(cur_indent), indent_(indent) {}

  void setAntiAlias(bool aa) override;
  void setDither(bool dither) override;
  void setDrawStyle(DlDrawStyle style) override;
  void setColor(DlColor color) override;
  void setStrokeWidth(DlScalar width) override;
  void setStrokeMiter(DlScalar limit) override;
  void setStrokeCap(DlStrokeCap cap) override;
  void setStrokeJoin(DlStrokeJoin join) override;
  void setColorSource(const DlColorSource* source) override;
  void setColorFilter(const DlColorFilter* filter) override;
  void setInvertColors(bool invert) override;
  void setBlendMode(DlBlendMode mode) override;
  void setPathEffect(const DlPathEffect* effect) override;
  void setMaskFilter(const DlMaskFilter* filter) override;
  void setImageFilter(const DlImageFilter* filter) override;

  void save() override;
  void saveLayer(const DlFRect* bounds,
                 const SaveLayerOptions options,
                 const DlImageFilter* backdrop) override;
  void restore() override;

  void translate(DlScalar tx, DlScalar ty) override;
  void scale(DlScalar sx, DlScalar sy) override;
  void rotate(DlAngle angle) override;
  void skew(DlScalar sx, DlScalar sy) override;
  // clang-format off
  void transform2DAffine(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                         DlScalar myx, DlScalar myy, DlScalar myt) override;
  void transformFullPerspective(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) override;
  // clang-format on
  void transformReset() override;

  void clipRect(const DlFRect& rect, ClipOp clip_op, bool is_aa) override;
  void clipRRect(const DlFRRect& rrect, ClipOp clip_op, bool is_aa) override;
  void clipPath(const DlPath& path, ClipOp clip_op, bool is_aa) override;

  void drawColor(DlColor color, DlBlendMode mode) override;
  void drawPaint() override;
  void drawLine(const DlFPoint& p0, const DlFPoint& p1) override;
  void drawRect(const DlFRect& rect) override;
  void drawOval(const DlFRect& bounds) override;
  void drawCircle(const DlFPoint& center, DlScalar radius) override;
  void drawRRect(const DlFRRect& rrect) override;
  void drawDRRect(const DlFRRect& outer, const DlFRRect& inner) override;
  void drawPath(const DlPath& path) override;
  void drawArc(const DlFRect& oval_bounds,
               DlScalar start_degrees,
               DlScalar sweep_degrees,
               bool use_center) override;
  void drawPoints(PointMode mode,
                  uint32_t count,
                  const DlFPoint points[]) override;
  void drawVertices(const DlVertices* vertices, DlBlendMode mode) override;
  void drawImage(const sk_sp<DlImage> image,
                 const DlFPoint point,
                 DlImageSampling sampling,
                 bool render_with_attributes) override;
  void drawImageRect(const sk_sp<DlImage> image,
                     const DlFRect& src,
                     const DlFRect& dst,
                     DlImageSampling sampling,
                     bool render_with_attributes,
                     SrcRectConstraint constraint) override;
  void drawImageNine(const sk_sp<DlImage> image,
                     const DlIRect& center,
                     const DlFRect& dst,
                     DlFilterMode filter,
                     bool render_with_attributes) override;
  void drawAtlas(const sk_sp<DlImage> atlas,
                 const DlRSTransform xform[],
                 const DlFRect tex[],
                 const DlColor colors[],
                 int count,
                 DlBlendMode mode,
                 DlImageSampling sampling,
                 const DlFRect* cull_rect,
                 bool render_with_attributes) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list,
                       DlScalar opacity) override;
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    DlScalar x,
                    DlScalar y) override;
  void drawShadow(const DlPath& path,
                  const DlColor color,
                  const DlScalar elevation,
                  bool transparent_occluder,
                  DlScalar dpr) override;

 private:
  std::ostream& os_;
  int cur_indent_;
  int indent_;

  void indent() { indent(indent_); }
  void outdent() { outdent(indent_); }
  void indent(int spaces) { cur_indent_ += spaces; }
  void outdent(int spaces) { cur_indent_ -= spaces; }

  template <class T>
  std::ostream& out_array(std::string name, int count, const T array[]);

  std::ostream& startl();

  void out(const DlColorFilter& filter);
  void out(const DlColorFilter* filter);
  void out(const DlImageFilter& filter);
  void out(const DlImageFilter* filter);
};

}  // namespace testing
}  // namespace flutter

#endif  // TESTING_DISPLAY_LIST_TESTING_H_
