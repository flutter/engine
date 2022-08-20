// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_PAINT_H_
#define FLUTTER_LIB_UI_PAINTING_PAINT_H_

#include <optional>

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/display_list_flags.h"
#include "flutter/display_list/display_list_mask_filter.h"
#include "flutter/display_list/display_list_paint.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/color_filter.h"
#include "flutter/lib/ui/painting/image_filter.h"
#include "flutter/lib/ui/painting/shader.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/third_party/tonic/dart_wrappable.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/tonic/converter/dart_converter.h"

namespace flutter {

class Paint : public RefCountedDartWrappable<Paint> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(Paint);

 public:
  ~Paint() override;

  static fml::RefPtr<Paint> Create(Dart_Handle wrapper, bool enable_dither) {
    UIDartState::ThrowIfUIOperationsProhibited();
    auto paint = fml::MakeRefCounted<Paint>(enable_dither);
    paint->AssociateWithDartWrapper(wrapper);
    return paint;
  }

  const SkPaint* paint(SkPaint& paint) const;

  /// Synchronize the Dart properties to the display list according
  /// to the attribute flags that indicate which properties are needed.
  /// The return value indicates if the paint was non-null and can
  /// either be DCHECKed or used to indicate to the DisplayList
  /// draw operation whether or not to use the synchronized attributes
  /// (mainly the drawImage and saveLayer methods).
  bool sync_to(DisplayListBuilder* builder,
               const DisplayListAttributeFlags& flags) const;

  void setIsAntiAlias(bool value) { is_anti_alias_ = value; }
  bool getIsAntiAlias() const { return is_anti_alias_; }

  void setIsInvertColors(bool value) { is_invert_colors_ = value; }
  bool getIsInvertColors() const { return is_invert_colors_; }

  uint32_t getColor() { return color_; }
  void setColor(uint32_t value) { color_ = value; }

  uint8_t getBlendMode() { return blend_mode_; }
  void setBlendMode(uint8_t value) { blend_mode_ = value; }

  uint8_t getStyle() { return style_; }
  void setStyle(uint8_t value) { style_ = value; }

  uint8_t getStrokeCap() { return stroke_cap_; }
  void setStrokeCap(uint8_t value) { stroke_cap_ = value; }

  uint8_t getStrokeJoin() { return stroke_join_; }
  void setStrokeJoin(uint8_t value) { stroke_join_ = value; }

  float getStrokeWidth() { return stroke_width_; }
  void setStrokeWidth(float value) { stroke_width_ = value; }

  float getStrokeMiter() { return stroke_miter_; }
  void setStrokeMiter(float value) { stroke_miter_ = value; }

  fml::RefPtr<Shader> getShader() { return shader_; }
  void setShader(fml::RefPtr<Shader> value, uint8_t filter_quality) {
    shader_ = value;
    filter_quality_ = filter_quality;
  }

  fml::RefPtr<ColorFilter> getColorFilter() { return color_filter_; }
  void setColorFilter(fml::RefPtr<ColorFilter> value) { color_filter_ = value; }

  fml::RefPtr<ImageFilter> getImageFilter() { return image_filter_; }
  void setImageFilter(fml::RefPtr<ImageFilter> value) { image_filter_ = value; }

  void clearMaskFilter() { mask_filter_ = std::nullopt; }
  void setBlurMaskFilter(uint8_t style, float sigma) {
    mask_filter_ = DlBlurMaskFilter(static_cast<SkBlurStyle>(style), sigma);
  }

 private:
  explicit Paint(bool enable_dither);
  bool is_dither_ = false;
  bool is_anti_alias_ = true;
  bool is_invert_colors_ = false;
  uint32_t color_ = 0xFF000000;
  uint8_t blend_mode_ = static_cast<uint8_t>(DlBlendMode::kSrcOver);
  uint8_t style_ = 0;
  uint8_t stroke_cap_ = 0;
  uint8_t stroke_join_ = 0;
  float stroke_width_ = 0.f;
  float stroke_miter_ = 4.f;
  uint8_t filter_quality_ = 0;
  fml::RefPtr<Shader> shader_ = nullptr;
  fml::RefPtr<ColorFilter> color_filter_ = nullptr;
  fml::RefPtr<ImageFilter> image_filter_ = nullptr;
  std::optional<DlBlurMaskFilter> mask_filter_ = std::nullopt;

  const std::shared_ptr<const DlColorFilter> dl_color_filter() const {
    if (!color_filter_) {
      return nullptr;
    }
    return color_filter_->filter();
  }

  sk_sp<SkColorFilter> sk_color_filter() const {
    auto color_filter = dl_color_filter();
    return color_filter ? color_filter->skia_object() : nullptr;
  }

  const std::shared_ptr<const DlImageFilter> dl_image_filter() const {
    if (!image_filter_) {
      return nullptr;
    }
    return image_filter_->filter();
  }

  sk_sp<SkImageFilter> sk_image_filter() const {
    auto image_filter = dl_image_filter();
    return image_filter ? image_filter->skia_object() : nullptr;
  }

  sk_sp<SkMaskFilter> sk_mask_filter() const {
    if (!mask_filter_) {
      return nullptr;
    }
    return mask_filter_->skia_object();
  }

  std::shared_ptr<const DlColorSource> dl_color_source() const {
    if (!shader_) {
      return nullptr;
    }
    auto sampling = ImageFilter::SamplingFromIndex(filter_quality_);
    return shader_->shader(sampling);
  }

  sk_sp<SkShader> sk_shader() const {
    auto color_source = dl_color_source();
    return color_source && color_source->owning_context() !=
                               DlImage::OwningContext::kRaster
               ? color_source->skia_object()
               : nullptr;
  }

  FML_DISALLOW_COPY_AND_ASSIGN(Paint);
};

}  // namespace flutter
#endif  // FLUTTER_LIB_UI_PAINTING_PAINT_H_
