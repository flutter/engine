// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DL_OP_RECORDS_H_
#define FLUTTER_DISPLAY_LIST_DL_OP_RECORDS_H_

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_op_receiver.h"
#include "flutter/display_list/dl_sampling_options.h"
#include "flutter/display_list/effects/dl_color_source.h"
#include "flutter/fml/macros.h"

#include "flutter/impeller/geometry/path.h"
#include "flutter/impeller/typographer/text_frame.h"
#include "third_party/skia/include/core/SkRSXform.h"

namespace flutter {

// Structure holding the information necessary to dispatch and
// potentially cull the DLOps during playback.
//
// Generally drawing ops will execute as long as |cur_index|
// is at or after |next_render_index|, so setting the latter
// to 0 will render all primitives and setting it to MAX_INT
// will skip all remaining rendering primitives.
//
// Save and saveLayer ops will execute as long as the next
// rendering index is before their closing restore index.
// They will also store their own restore index into the
// |next_restore_index| field for use by clip and transform ops.
//
// Clip and transform ops will only execute if the next
// render index is before the next restore index. Otherwise
// their modified state will not be used before it gets
// restored.
//
// Attribute ops always execute as they are too numerous and
// cheap to deal with a complicated "lifetime" tracking to
// determine if they will be used.
struct DispatchContext {
  DlOpReceiver& receiver;

  int cur_index;
  int next_render_index;

  int next_restore_index;

  struct SaveInfo {
    SaveInfo(int previous_restore_index, bool save_was_needed)
        : previous_restore_index(previous_restore_index),
          save_was_needed(save_was_needed) {}

    int previous_restore_index;
    bool save_was_needed;
  };

  std::vector<SaveInfo> save_infos;
};

// Most Ops can be bulk compared using memcmp because they contain
// only numeric values or constructs that are constructed from numeric
// values.
//
// Some contain sk_sp<> references which can also be bulk compared
// to see if they are pointing to the same reference. (Note that
// two sk_sp<> that refer to the same object are themselves ==.)
//
// Only a DLOp that wants to do a deep compare needs to override the
// DLOp::equals() method and return a value of kEqual or kNotEqual.
enum class DisplayListCompare {
  // The Op is deferring comparisons to a bulk memcmp performed lazily
  // across all bulk-comparable ops.
  kUseBulkCompare,

  // The Op provided a specific equals method that spotted a difference
  kNotEqual,

  // The Op provided a specific equals method that saw no differences
  kEqual,
};

// "DLOpPackLabel" is just a label for the pack pragma so it can be popped
// later.
#pragma pack(push, DLOpPackLabel, 8)

// Assuming a 64-bit platform (most of our platforms at this time?)
// the following comments are a "worst case" assessment of how well
// these structures pack into memory. They may be packed more tightly
// on some of the 32-bit platforms that we see in older phones.
//
// Struct allocation in the DL memory is aligned to a void* boundary
// which means that the minimum (aligned) struct size will be 8 bytes.
// The DLOp base uses 4 bytes so each Op-specific struct gets 4 bytes
// of data for "free" and works best when it packs well into an 8-byte
// aligned size.
struct DLOp {
  static constexpr uint32_t kDepthInc = 0;
  static constexpr uint32_t kRenderOpInc = 0;

  DisplayListOpType type : 8;
  uint32_t size : 24;

  DisplayListCompare equals(const DLOp* other) const {
    return DisplayListCompare::kUseBulkCompare;
  }
};

// 4 byte header + 4 byte payload packs into minimum 8 bytes
#define DEFINE_SET_BOOL_OP(name)                                 \
  struct Set##name##Op final : DLOp {                            \
    static constexpr auto kType = DisplayListOpType::kSet##name; \
                                                                 \
    explicit Set##name##Op(bool value) : value(value) {}         \
                                                                 \
    const bool value;                                            \
                                                                 \
    void dispatch(DispatchContext& ctx) const {                  \
      ctx.receiver.set##name(value);                             \
    }                                                            \
  };
DEFINE_SET_BOOL_OP(AntiAlias)
DEFINE_SET_BOOL_OP(InvertColors)
#undef DEFINE_SET_BOOL_OP
static_assert(sizeof(SetAntiAliasOp) == 8);

// 4 byte header + 4 byte payload packs into minimum 8 bytes
#define DEFINE_SET_ENUM_OP(name)                                         \
  struct SetStroke##name##Op final : DLOp {                              \
    static constexpr auto kType = DisplayListOpType::kSetStroke##name;   \
                                                                         \
    explicit SetStroke##name##Op(DlStroke##name value) : value(value) {} \
                                                                         \
    const DlStroke##name value;                                          \
                                                                         \
    void dispatch(DispatchContext& ctx) const {                          \
      ctx.receiver.setStroke##name(value);                               \
    }                                                                    \
  };
DEFINE_SET_ENUM_OP(Cap)
DEFINE_SET_ENUM_OP(Join)
#undef DEFINE_SET_ENUM_OP
static_assert(sizeof(SetStrokeCapOp) == 8);

// 4 byte header + 4 byte payload packs into minimum 8 bytes
struct SetStyleOp final : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetStyle;

  explicit SetStyleOp(DlDrawStyle style) : style(style) {}

  const DlDrawStyle style;

  void dispatch(DispatchContext& ctx) const {
    ctx.receiver.setDrawStyle(style);
  }
};
static_assert(sizeof(SetStyleOp) == 8);

// 4 byte header + 4 byte payload packs into minimum 8 bytes
struct SetStrokeWidthOp final : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetStrokeWidth;

  explicit SetStrokeWidthOp(float width) : width(width) {}

  const float width;

  void dispatch(DispatchContext& ctx) const {
    ctx.receiver.setStrokeWidth(width);
  }
};
static_assert(sizeof(SetStrokeWidthOp) == 8);

// 4 byte header + 4 byte payload packs into minimum 8 bytes
struct SetStrokeMiterOp final : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetStrokeMiter;

  explicit SetStrokeMiterOp(float limit) : limit(limit) {}

  const float limit;

  void dispatch(DispatchContext& ctx) const {
    ctx.receiver.setStrokeMiter(limit);
  }
};
static_assert(sizeof(SetStrokeMiterOp) == 8);

// 4 byte header + 4 byte payload packs into minimum 8 bytes
struct SetColorOp final : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetColor;

  explicit SetColorOp(DlColor color) : color(color) {}

  const DlColor color;

  void dispatch(DispatchContext& ctx) const { ctx.receiver.setColor(color); }
};
static_assert(sizeof(SetColorOp) == 8);

// 4 byte header + 4 byte payload packs into minimum 8 bytes
struct SetBlendModeOp final : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetBlendMode;

  explicit SetBlendModeOp(DlBlendMode mode) : mode(mode) {}

  const DlBlendMode mode;

  void dispatch(DispatchContext& ctx) const {  //
    ctx.receiver.setBlendMode(mode);
  }
};
static_assert(sizeof(SetBlendModeOp) == 8);

// Clear: 4 byte header + unused 4 byte payload uses 8 bytes
//        (4 bytes unused)
// Set: 4 byte header + unused 4 byte struct padding + Dl<name>
//      instance copied to the memory following the record
//      yields a size and efficiency that has somewhere between
//      4 and 8 bytes unused
#define DEFINE_SET_CLEAR_DLATTR_OP(name, sk_name, field)                    \
  struct Clear##name##Op final : DLOp {                                     \
    static constexpr auto kType = DisplayListOpType::kClear##name;          \
                                                                            \
    Clear##name##Op() {}                                                    \
                                                                            \
    void dispatch(DispatchContext& ctx) const {                             \
      ctx.receiver.set##name(nullptr);                                      \
    }                                                                       \
  };                                                                        \
  struct SetPod##name##Op final : DLOp {                                    \
    static constexpr auto kType = DisplayListOpType::kSetPod##name;         \
                                                                            \
    SetPod##name##Op() {}                                                   \
                                                                            \
    void dispatch(DispatchContext& ctx) const {                             \
      const Dl##name* filter = reinterpret_cast<const Dl##name*>(this + 1); \
      ctx.receiver.set##name(filter);                                       \
    }                                                                       \
  };
DEFINE_SET_CLEAR_DLATTR_OP(ColorFilter, ColorFilter, filter)
DEFINE_SET_CLEAR_DLATTR_OP(ImageFilter, ImageFilter, filter)
DEFINE_SET_CLEAR_DLATTR_OP(MaskFilter, MaskFilter, filter)
DEFINE_SET_CLEAR_DLATTR_OP(ColorSource, Shader, source)
#undef DEFINE_SET_CLEAR_DLATTR_OP
static_assert(sizeof(ClearColorFilterOp) == 4);
static_assert(sizeof(SetPodColorFilterOp) == 4);

// 4 byte header + 88 bytes for the embedded DlImageColorSource
// uses 96 total bytes (4 bytes unused)
struct SetImageColorSourceOp : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetImageColorSource;

  explicit SetImageColorSourceOp(const DlImageColorSource* source)
      : source(source->image(),
               source->horizontal_tile_mode(),
               source->vertical_tile_mode(),
               source->sampling(),
               source->matrix_ptr()) {}

  const DlImageColorSource source;

  void dispatch(DispatchContext& ctx) const {
    ctx.receiver.setColorSource(&source);
  }
};
static_assert(sizeof(SetImageColorSourceOp) == 96);

struct SetRuntimeEffectColorSourceOp : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetRuntimeEffectColorSource;

  explicit SetRuntimeEffectColorSourceOp(
      const DlRuntimeEffectColorSource* source)
      : source(source->runtime_effect(),
               source->samplers(),
               source->uniform_data()) {}

  const DlRuntimeEffectColorSource source;

  void dispatch(DispatchContext& ctx) const {
    ctx.receiver.setColorSource(&source);
  }

  DisplayListCompare equals(const SetRuntimeEffectColorSourceOp* other) const {
    return (source == other->source) ? DisplayListCompare::kEqual
                                     : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(SetRuntimeEffectColorSourceOp) == 64);

#ifdef IMPELLER_ENABLE_3D
struct SetSceneColorSourceOp : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetSceneColorSource;

  explicit SetSceneColorSourceOp(const DlSceneColorSource* source)
      : source(source->scene_node(), source->camera_matrix()) {}

  const DlSceneColorSource source;

  void dispatch(DispatchContext& ctx) const {
    ctx.receiver.setColorSource(&source);
  }

  DisplayListCompare equals(const SetSceneColorSourceOp* other) const {
    return (source == other->source) ? DisplayListCompare::kEqual
                                     : DisplayListCompare::kNotEqual;
  }
};
#endif  // IMPELLER_ENABLE_3D

// 4 byte header + 16 byte payload uses 24 total bytes (4 bytes unused)
struct SetSharedImageFilterOp : DLOp {
  static constexpr auto kType = DisplayListOpType::kSetSharedImageFilter;

  explicit SetSharedImageFilterOp(const DlImageFilter* filter)
      : filter(filter->shared()) {}

  const std::shared_ptr<DlImageFilter> filter;

  void dispatch(DispatchContext& ctx) const {
    ctx.receiver.setImageFilter(filter.get());
  }

  DisplayListCompare equals(const SetSharedImageFilterOp* other) const {
    return Equals(filter, other->filter) ? DisplayListCompare::kEqual
                                         : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(SetSharedImageFilterOp) == 24);

// The base struct for all save() and saveLayer() ops
// 4 byte header + 12 byte payload packs exactly into 16 bytes
struct SaveOpBase : DLOp {
  static constexpr uint32_t kDepthInc = 0;
  static constexpr uint32_t kRenderOpInc = 1;

  SaveOpBase() : options(), restore_index(0) {}

  explicit SaveOpBase(const SaveLayerOptions& options)
      : options(options), restore_index(0), total_content_depth(0) {}

  // options parameter is only used by saveLayer operations, but since
  // it packs neatly into the empty space created by laying out the rest
  // of the data here, it can be stored for free and defaulted to 0 for
  // save operations.
  SaveLayerOptions options;
  int restore_index;
  uint32_t total_content_depth;

  inline bool save_needed(DispatchContext& ctx) const {
    bool needed = ctx.next_render_index <= restore_index;
    ctx.save_infos.emplace_back(ctx.next_restore_index, needed);
    ctx.next_restore_index = restore_index;
    return needed;
  }
};
static_assert(sizeof(SaveOpBase) == 16);

// 16 byte SaveOpBase with no additional data (options is unsed here)
struct SaveOp final : SaveOpBase {
  static constexpr auto kType = DisplayListOpType::kSave;

  SaveOp() : SaveOpBase() {}

  void dispatch(DispatchContext& ctx) const {
    if (save_needed(ctx)) {
      ctx.receiver.save(total_content_depth);
    }
  }
};
static_assert(sizeof(SaveOp) == 16);

// The base struct for all saveLayer() ops
// 16 byte SaveOpBase + 20 byte payload packs into 36 bytes
struct SaveLayerOpBase : SaveOpBase {
  SaveLayerOpBase(const SaveLayerOptions& options, const SkRect& rect)
      : SaveOpBase(options), rect(rect) {}

  SkRect rect;
  DlBlendMode max_blend_mode = DlBlendMode::kClear;
};
static_assert(sizeof(SaveLayerOpBase) == 36);

// 36 byte SaveLayerOpBase with no additional data packs into 40 bytes
// of buffer storage with 4 bytes unused.
struct SaveLayerOp final : SaveLayerOpBase {
  static constexpr auto kType = DisplayListOpType::kSaveLayer;

  SaveLayerOp(const SaveLayerOptions& options, const SkRect& rect)
      : SaveLayerOpBase(options, rect) {}

  void dispatch(DispatchContext& ctx) const {
    if (save_needed(ctx)) {
      ctx.receiver.saveLayer(rect, options, total_content_depth,
                             max_blend_mode);
    }
  }
};
static_assert(sizeof(SaveLayerOp) == 36);

// 36 byte SaveLayerOpBase + 4 bytes for alignment + 16 byte payload packs
// into minimum 56 bytes
struct SaveLayerBackdropOp final : SaveLayerOpBase {
  static constexpr auto kType = DisplayListOpType::kSaveLayerBackdrop;

  SaveLayerBackdropOp(const SaveLayerOptions& options,
                      const SkRect& rect,
                      const DlImageFilter* backdrop)
      : SaveLayerOpBase(options, rect), backdrop(backdrop->shared()) {}

  const std::shared_ptr<DlImageFilter> backdrop;

  void dispatch(DispatchContext& ctx) const {
    if (save_needed(ctx)) {
      ctx.receiver.saveLayer(rect, options, total_content_depth, max_blend_mode,
                             backdrop.get());
    }
  }

  DisplayListCompare equals(const SaveLayerBackdropOp* other) const {
    return (options == other->options && rect == other->rect &&
            Equals(backdrop, other->backdrop))
               ? DisplayListCompare::kEqual
               : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(SaveLayerBackdropOp) == 56);

// 4 byte header + no payload uses minimum 8 bytes (4 bytes unused)
struct RestoreOp final : DLOp {
  static constexpr auto kType = DisplayListOpType::kRestore;
  static constexpr uint32_t kDepthInc = 0;
  static constexpr uint32_t kRenderOpInc = 1;

  RestoreOp() {}

  void dispatch(DispatchContext& ctx) const {
    DispatchContext::SaveInfo& info = ctx.save_infos.back();
    if (info.save_was_needed) {
      ctx.receiver.restore();
    }
    ctx.next_restore_index = info.previous_restore_index;
    ctx.save_infos.pop_back();
  }
};
static_assert(sizeof(RestoreOp) == 4);

struct TransformClipOpBase : DLOp {
  static constexpr uint32_t kDepthInc = 0;
  static constexpr uint32_t kRenderOpInc = 1;

  inline bool op_needed(const DispatchContext& context) const {
    return context.next_render_index <= context.next_restore_index;
  }
};
static_assert(sizeof(TransformClipOpBase) == 4);

// 4 byte header + 8 byte payload uses 12 bytes but is rounded up to 16 bytes
// (4 bytes unused)
struct TranslateOp final : TransformClipOpBase {
  static constexpr auto kType = DisplayListOpType::kTranslate;

  TranslateOp(SkScalar tx, SkScalar ty) : tx(tx), ty(ty) {}

  const SkScalar tx;
  const SkScalar ty;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.translate(tx, ty);
    }
  }
};
static_assert(sizeof(TranslateOp) == 12);

// 4 byte header + 8 byte payload uses 12 bytes but is rounded up to 16 bytes
// (4 bytes unused)
struct ScaleOp final : TransformClipOpBase {
  static constexpr auto kType = DisplayListOpType::kScale;

  ScaleOp(SkScalar sx, SkScalar sy) : sx(sx), sy(sy) {}

  const SkScalar sx;
  const SkScalar sy;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.scale(sx, sy);
    }
  }
};
static_assert(sizeof(ScaleOp) == 12);

// 4 byte header + 4 byte payload packs into minimum 8 bytes
struct RotateOp final : TransformClipOpBase {
  static constexpr auto kType = DisplayListOpType::kRotate;

  explicit RotateOp(SkScalar degrees) : degrees(degrees) {}

  const SkScalar degrees;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.rotate(degrees);
    }
  }
};
static_assert(sizeof(RotateOp) == 8);

// 4 byte header + 8 byte payload uses 12 bytes but is rounded up to 16 bytes
// (4 bytes unused)
struct SkewOp final : TransformClipOpBase {
  static constexpr auto kType = DisplayListOpType::kSkew;

  SkewOp(SkScalar sx, SkScalar sy) : sx(sx), sy(sy) {}

  const SkScalar sx;
  const SkScalar sy;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.skew(sx, sy);
    }
  }
};
static_assert(sizeof(SkewOp) == 12);

// 4 byte header + 24 byte payload uses 28 bytes but is rounded up to 32 bytes
// (4 bytes unused)
struct Transform2DAffineOp final : TransformClipOpBase {
  static constexpr auto kType = DisplayListOpType::kTransform2DAffine;

  // clang-format off
  Transform2DAffineOp(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                      SkScalar myx, SkScalar myy, SkScalar myt)
      : mxx(mxx), mxy(mxy), mxt(mxt), myx(myx), myy(myy), myt(myt) {}
  // clang-format on

  const SkScalar mxx, mxy, mxt;
  const SkScalar myx, myy, myt;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.transform2DAffine(mxx, mxy, mxt,  //
                                     myx, myy, myt);
    }
  }
};
static_assert(sizeof(Transform2DAffineOp) == 28);

// 4 byte header + 64 byte payload uses 68 bytes which is rounded up to 72 bytes
// (4 bytes unused)
struct TransformFullPerspectiveOp final : TransformClipOpBase {
  static constexpr auto kType = DisplayListOpType::kTransformFullPerspective;

  // clang-format off
  TransformFullPerspectiveOp(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt)
      : mxx(mxx), mxy(mxy), mxz(mxz), mxt(mxt),
        myx(myx), myy(myy), myz(myz), myt(myt),
        mzx(mzx), mzy(mzy), mzz(mzz), mzt(mzt),
        mwx(mwx), mwy(mwy), mwz(mwz), mwt(mwt) {}
  // clang-format on

  const SkScalar mxx, mxy, mxz, mxt;
  const SkScalar myx, myy, myz, myt;
  const SkScalar mzx, mzy, mzz, mzt;
  const SkScalar mwx, mwy, mwz, mwt;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.transformFullPerspective(mxx, mxy, mxz, mxt,  //
                                            myx, myy, myz, myt,  //
                                            mzx, mzy, mzz, mzt,  //
                                            mwx, mwy, mwz, mwt);
    }
  }
};
static_assert(sizeof(TransformFullPerspectiveOp) == 68);

// 4 byte header with no payload.
struct TransformResetOp final : TransformClipOpBase {
  static constexpr auto kType = DisplayListOpType::kTransformReset;

  TransformResetOp() = default;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.transformReset();
    }
  }
};
static_assert(sizeof(TransformResetOp) == 4);

// 4 byte header + 4 byte common payload packs into minimum 8 bytes
// SkRect is 16 more bytes, which packs efficiently into 24 bytes total
// SkRRect is 52 more bytes, which rounds up to 56 bytes (4 bytes unused)
//         which packs into 64 bytes total
// CacheablePath is 128 more bytes, which packs efficiently into 136 bytes total
//
// We could pack the clip_op and the bool both into the free 4 bytes after
// the header, but the Windows compiler keeps wanting to expand that
// packing into more bytes than needed (even when they are declared as
// packed bit fields!)
#define DEFINE_CLIP_SHAPE_OP(shapetype, clipop)                                \
  struct Clip##clipop##shapetype##Op final : TransformClipOpBase {             \
    static constexpr auto kType = DisplayListOpType::kClip##clipop##shapetype; \
                                                                               \
    Clip##clipop##shapetype##Op(Sk##shapetype shape, bool is_aa)               \
        : is_aa(is_aa), shape(shape) {}                                        \
                                                                               \
    const bool is_aa;                                                          \
    const Sk##shapetype shape;                                                 \
                                                                               \
    void dispatch(DispatchContext& ctx) const {                                \
      if (op_needed(ctx)) {                                                    \
        ctx.receiver.clip##shapetype(shape, DlCanvas::ClipOp::k##clipop,       \
                                     is_aa);                                   \
      }                                                                        \
    }                                                                          \
  };
DEFINE_CLIP_SHAPE_OP(Rect, Intersect)
DEFINE_CLIP_SHAPE_OP(RRect, Intersect)
DEFINE_CLIP_SHAPE_OP(Rect, Difference)
DEFINE_CLIP_SHAPE_OP(RRect, Difference)
#undef DEFINE_CLIP_SHAPE_OP
static_assert(sizeof(ClipIntersectRectOp) == 24);
static_assert(sizeof(ClipIntersectRRectOp) == 60);

#define DEFINE_CLIP_PATH_OP(clipop)                                       \
  struct Clip##clipop##PathOp final : TransformClipOpBase {               \
    static constexpr auto kType = DisplayListOpType::kClip##clipop##Path; \
                                                                          \
    Clip##clipop##PathOp(const SkPath& path, bool is_aa)                  \
        : is_aa(is_aa), cached_path(path) {}                              \
                                                                          \
    const bool is_aa;                                                     \
    const DlOpReceiver::CacheablePath cached_path;                        \
                                                                          \
    void dispatch(DispatchContext& ctx) const {                           \
      if (op_needed(ctx)) {                                               \
        if (ctx.receiver.PrefersImpellerPaths()) {                        \
          ctx.receiver.clipPath(cached_path, DlCanvas::ClipOp::k##clipop, \
                                is_aa);                                   \
        } else {                                                          \
          ctx.receiver.clipPath(cached_path.sk_path,                      \
                                DlCanvas::ClipOp::k##clipop, is_aa);      \
        }                                                                 \
      }                                                                   \
    }                                                                     \
                                                                          \
    DisplayListCompare equals(const Clip##clipop##PathOp* other) const {  \
      return is_aa == other->is_aa && cached_path == other->cached_path   \
                 ? DisplayListCompare::kEqual                             \
                 : DisplayListCompare::kNotEqual;                         \
    }                                                                     \
  };
DEFINE_CLIP_PATH_OP(Intersect)
DEFINE_CLIP_PATH_OP(Difference)
#undef DEFINE_CLIP_PATH_OP
static_assert(sizeof(ClipIntersectPathOp) == 40);

struct DrawOpBase : DLOp {
  static constexpr uint32_t kDepthInc = 1;
  static constexpr uint32_t kRenderOpInc = 1;

  inline bool op_needed(const DispatchContext& ctx) const {
    return ctx.cur_index >= ctx.next_render_index;
  }
};
static_assert(sizeof(DrawOpBase) == 4);

// 4 byte header + no payload uses minimum 8 bytes (4 bytes unused)
struct DrawPaintOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawPaint;

  DrawPaintOp() {}

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawPaint();
    }
  }
};
static_assert(sizeof(DrawPaintOp) == 4);

// 4 byte header + 8 byte payload uses 12 bytes but is rounded up to 16 bytes
// (4 bytes unused)
struct DrawColorOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawColor;

  DrawColorOp(DlColor color, DlBlendMode mode) : color(color), mode(mode) {}

  const DlColor color;
  const DlBlendMode mode;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawColor(color, mode);
    }
  }
};
static_assert(sizeof(DrawColorOp) == 12);

// The common data is a 4 byte header with an unused 4 bytes
// SkRect is 16 more bytes, using 20 bytes which rounds up to 24 bytes total
//        (4 bytes unused)
// SkOval is same as SkRect
// SkRRect is 52 more bytes, which packs efficiently into 56 bytes total
#define DEFINE_DRAW_1ARG_OP(op_name, arg_type, arg_name)                  \
  struct Draw##op_name##Op final : DrawOpBase {                           \
    static constexpr auto kType = DisplayListOpType::kDraw##op_name;      \
                                                                          \
    explicit Draw##op_name##Op(arg_type arg_name) : arg_name(arg_name) {} \
                                                                          \
    const arg_type arg_name;                                              \
                                                                          \
    void dispatch(DispatchContext& ctx) const {                           \
      if (op_needed(ctx)) {                                               \
        ctx.receiver.draw##op_name(arg_name);                             \
      }                                                                   \
    }                                                                     \
  };
DEFINE_DRAW_1ARG_OP(Rect, SkRect, rect)
DEFINE_DRAW_1ARG_OP(Oval, SkRect, oval)
DEFINE_DRAW_1ARG_OP(RRect, SkRRect, rrect)
#undef DEFINE_DRAW_1ARG_OP
static_assert(sizeof(DrawRectOp) == 20);
static_assert(sizeof(DrawOvalOp) == 20);
static_assert(sizeof(DrawRRectOp) == 56);

// 4 byte header + 32 byte payload + 4 byte padding
struct DrawPathOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawPath;

  explicit DrawPathOp(const SkPath& path) : cached_path(path) {}

  const DlOpReceiver::CacheablePath cached_path;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      if (ctx.receiver.PrefersImpellerPaths()) {
        ctx.receiver.drawPath(cached_path);
      } else {
        ctx.receiver.drawPath(cached_path.sk_path);
      }
    }
  }

  DisplayListCompare equals(const DrawPathOp* other) const {
    return cached_path == other->cached_path ? DisplayListCompare::kEqual
                                             : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(DrawPathOp) == 40);

// The common data is a 4 byte header with an unused 4 bytes
// 2 x SkPoint is 16 more bytes, using 20 bytes rounding up to 24 bytes total
//             (4 bytes unused)
// SkPoint + SkScalar is 12 more bytes, packing efficiently into 16 bytes total
// 2 x SkRRect is 104 more bytes, using 108 and rounding up to 112 bytes total
//             (4 bytes unused)
#define DEFINE_DRAW_2ARG_OP(op_name, type1, name1, type2, name2)     \
  struct Draw##op_name##Op final : DrawOpBase {                      \
    static constexpr auto kType = DisplayListOpType::kDraw##op_name; \
                                                                     \
    Draw##op_name##Op(type1 name1, type2 name2)                      \
        : name1(name1), name2(name2) {}                              \
                                                                     \
    const type1 name1;                                               \
    const type2 name2;                                               \
                                                                     \
    void dispatch(DispatchContext& ctx) const {                      \
      if (op_needed(ctx)) {                                          \
        ctx.receiver.draw##op_name(name1, name2);                    \
      }                                                              \
    }                                                                \
  };
DEFINE_DRAW_2ARG_OP(Line, SkPoint, p0, SkPoint, p1)
DEFINE_DRAW_2ARG_OP(Circle, SkPoint, center, SkScalar, radius)
DEFINE_DRAW_2ARG_OP(DRRect, SkRRect, outer, SkRRect, inner)
#undef DEFINE_DRAW_2ARG_OP
static_assert(sizeof(DrawLineOp) == 20);
static_assert(sizeof(DrawCircleOp) == 16);
static_assert(sizeof(DrawDRRectOp) == 108);

// 4 byte header + 24 byte payload packs into 32 bytes (4 bytes unused)
struct DrawDashedLineOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawDashedLine;

  DrawDashedLineOp(const DlPoint& p0,
                   const DlPoint& p1,
                   DlScalar on_length,
                   DlScalar off_length)
      : p0(p0), p1(p1), on_length(on_length), off_length(off_length) {}

  const DlPoint p0;
  const DlPoint p1;
  const SkScalar on_length;
  const SkScalar off_length;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawDashedLine(p0, p1, on_length, off_length);
    }
  }
};
static_assert(sizeof(DrawDashedLineOp) == 28);

// 4 byte header + 28 byte payload packs efficiently into 32 bytes
struct DrawArcOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawArc;

  DrawArcOp(SkRect bounds, SkScalar start, SkScalar sweep, bool center)
      : bounds(bounds), start(start), sweep(sweep), center(center) {}

  const SkRect bounds;
  const SkScalar start;
  const SkScalar sweep;
  const bool center;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawArc(bounds, start, sweep, center);
    }
  }
};
static_assert(sizeof(DrawArcOp) == 32);

// 4 byte header + 4 byte fixed payload packs efficiently into 8 bytes
// But then there is a list of points following the structure which
// is guaranteed to be a multiple of 8 bytes (SkPoint is 8 bytes)
// so this op will always pack efficiently
// The point type is packed into 3 different OpTypes to avoid expanding
// the fixed payload beyond the 8 bytes
#define DEFINE_DRAW_POINTS_OP(name, mode)                                \
  struct Draw##name##Op final : DrawOpBase {                             \
    static constexpr auto kType = DisplayListOpType::kDraw##name;        \
                                                                         \
    explicit Draw##name##Op(uint32_t count) : count(count) {}            \
                                                                         \
    const uint32_t count;                                                \
                                                                         \
    void dispatch(DispatchContext& ctx) const {                          \
      if (op_needed(ctx)) {                                              \
        const SkPoint* pts = reinterpret_cast<const SkPoint*>(this + 1); \
        ctx.receiver.drawPoints(DlCanvas::PointMode::mode, count, pts);  \
      }                                                                  \
    }                                                                    \
  };
DEFINE_DRAW_POINTS_OP(Points, kPoints);
DEFINE_DRAW_POINTS_OP(Lines, kLines);
DEFINE_DRAW_POINTS_OP(Polygon, kPolygon);
#undef DEFINE_DRAW_POINTS_OP
static_assert(sizeof(DrawPointsOp) == 8);
static_assert(sizeof(DrawLinesOp) == 8);
static_assert(sizeof(DrawPolygonOp) == 8);

// 4 byte header + 4 byte payload packs efficiently into 8 bytes
// The DlVertices object will be pod-allocated after this structure
// and can take any number of bytes so the final efficiency will
// depend on the size of the DlVertices.
// Note that the DlVertices object ends with an array of 16-bit
// indices so the alignment can be up to 6 bytes off leading to
// up to 6 bytes of overhead
struct DrawVerticesOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawVertices;

  explicit DrawVerticesOp(DlBlendMode mode) : mode(mode) {}

  const DlBlendMode mode;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      const DlVertices* vertices =
          reinterpret_cast<const DlVertices*>(this + 1);
      ctx.receiver.drawVertices(vertices, mode);
    }
  }
};
static_assert(sizeof(DrawVerticesOp) == 8);

#define DEFINE_DRAW_IMAGE_OP(name, with_attributes)                      \
  struct name##Op final : DrawOpBase {                                   \
    static constexpr auto kType = DisplayListOpType::k##name;            \
                                                                         \
    name##Op(const sk_sp<DlImage>& image,                                \
             const SkPoint& point,                                       \
             DlImageSampling sampling)                                   \
        : point(point), sampling(sampling), image(std::move(image)) {}   \
                                                                         \
    const SkPoint point;                                                 \
    const DlImageSampling sampling;                                      \
    const sk_sp<DlImage> image;                                          \
                                                                         \
    void dispatch(DispatchContext& ctx) const {                          \
      if (op_needed(ctx)) {                                              \
        ctx.receiver.drawImage(image, point, sampling, with_attributes); \
      }                                                                  \
    }                                                                    \
                                                                         \
    DisplayListCompare equals(const name##Op* other) const {             \
      return (point == other->point && sampling == other->sampling &&    \
              image->Equals(other->image))                               \
                 ? DisplayListCompare::kEqual                            \
                 : DisplayListCompare::kNotEqual;                        \
    }                                                                    \
  };
DEFINE_DRAW_IMAGE_OP(DrawImage, false)
DEFINE_DRAW_IMAGE_OP(DrawImageWithAttr, true)
#undef DEFINE_DRAW_IMAGE_OP
static_assert(sizeof(DrawImageOp) == 24);

struct DrawImageRectOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawImageRect;

  DrawImageRectOp(const sk_sp<DlImage>& image,
                  const SkRect& src,
                  const SkRect& dst,
                  DlImageSampling sampling,
                  bool render_with_attributes,
                  DlCanvas::SrcRectConstraint constraint)
      : src(src),
        dst(dst),
        sampling(sampling),
        render_with_attributes(render_with_attributes),
        constraint(constraint),
        image(image) {}

  const SkRect src;
  const SkRect dst;
  const DlImageSampling sampling;
  const bool render_with_attributes;
  const DlCanvas::SrcRectConstraint constraint;
  const sk_sp<DlImage> image;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawImageRect(image, src, dst, sampling,
                                 render_with_attributes, constraint);
    }
  }

  DisplayListCompare equals(const DrawImageRectOp* other) const {
    return (src == other->src && dst == other->dst &&
            sampling == other->sampling &&
            render_with_attributes == other->render_with_attributes &&
            constraint == other->constraint && image->Equals(other->image))
               ? DisplayListCompare::kEqual
               : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(DrawImageRectOp) == 56);

// 4 byte header + 44 byte payload packs efficiently into 48 bytes
#define DEFINE_DRAW_IMAGE_NINE_OP(name, render_with_attributes)            \
  struct name##Op final : DrawOpBase {                                     \
    static constexpr auto kType = DisplayListOpType::k##name;              \
                                                                           \
    name##Op(const sk_sp<DlImage>& image,                                  \
             const SkIRect& center,                                        \
             const SkRect& dst,                                            \
             DlFilterMode mode)                                            \
        : center(center), dst(dst), mode(mode), image(std::move(image)) {} \
                                                                           \
    const SkIRect center;                                                  \
    const SkRect dst;                                                      \
    const DlFilterMode mode;                                               \
    const sk_sp<DlImage> image;                                            \
                                                                           \
    void dispatch(DispatchContext& ctx) const {                            \
      if (op_needed(ctx)) {                                                \
        ctx.receiver.drawImageNine(image, center, dst, mode,               \
                                   render_with_attributes);                \
      }                                                                    \
    }                                                                      \
                                                                           \
    DisplayListCompare equals(const name##Op* other) const {               \
      return (center == other->center && dst == other->dst &&              \
              mode == other->mode && image->Equals(other->image))          \
                 ? DisplayListCompare::kEqual                              \
                 : DisplayListCompare::kNotEqual;                          \
    }                                                                      \
  };
DEFINE_DRAW_IMAGE_NINE_OP(DrawImageNine, false)
DEFINE_DRAW_IMAGE_NINE_OP(DrawImageNineWithAttr, true)
#undef DEFINE_DRAW_IMAGE_NINE_OP
static_assert(sizeof(DrawImageNineOp) == 48);

// Each of these is then followed by a number of lists.
// SkRSXform list is a multiple of 16 bytes so it is always packed well
// SkRect list is also a multiple of 16 bytes so it also packs well
// DlColor list only packs well if the count is even, otherwise there
// can be 4 unusued bytes at the end.
struct DrawAtlasBaseOp : DrawOpBase {
  DrawAtlasBaseOp(const sk_sp<DlImage>& atlas,
                  int count,
                  DlBlendMode mode,
                  DlImageSampling sampling,
                  bool has_colors,
                  bool render_with_attributes)
      : count(count),
        mode_index(static_cast<uint16_t>(mode)),
        has_colors(has_colors),
        render_with_attributes(render_with_attributes),
        sampling(sampling),
        atlas(atlas) {}

  const int count;
  const uint16_t mode_index;
  const uint8_t has_colors;
  const uint8_t render_with_attributes;
  const DlImageSampling sampling;
  const sk_sp<DlImage> atlas;

  bool equals(const DrawAtlasBaseOp* other,
              const void* pod_this,
              const void* pod_other) const {
    bool ret = (count == other->count && mode_index == other->mode_index &&
                has_colors == other->has_colors &&
                render_with_attributes == other->render_with_attributes &&
                sampling == other->sampling && atlas->Equals(other->atlas));
    if (ret) {
      size_t bytes = count * (sizeof(SkRSXform) + sizeof(SkRect));
      if (has_colors) {
        bytes += count * sizeof(DlColor);
      }
      ret = (memcmp(pod_this, pod_other, bytes) == 0);
    }
    return ret;
  }
};
static_assert(sizeof(DrawAtlasBaseOp) == 24);

// Packs into 24 bytes as per DrawAtlasBaseOp
// with array data following the struct also as per DrawAtlasBaseOp
struct DrawAtlasOp final : DrawAtlasBaseOp {
  static constexpr auto kType = DisplayListOpType::kDrawAtlas;

  DrawAtlasOp(const sk_sp<DlImage>& atlas,
              int count,
              DlBlendMode mode,
              DlImageSampling sampling,
              bool has_colors,
              bool render_with_attributes)
      : DrawAtlasBaseOp(atlas,
                        count,
                        mode,
                        sampling,
                        has_colors,
                        render_with_attributes) {}

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      const SkRSXform* xform = reinterpret_cast<const SkRSXform*>(this + 1);
      const SkRect* tex = reinterpret_cast<const SkRect*>(xform + count);
      const DlColor* colors =
          has_colors ? reinterpret_cast<const DlColor*>(tex + count) : nullptr;
      const DlBlendMode mode = static_cast<DlBlendMode>(mode_index);
      ctx.receiver.drawAtlas(atlas, xform, tex, colors, count, mode, sampling,
                             nullptr, render_with_attributes);
    }
  }

  DisplayListCompare equals(const DrawAtlasOp* other) const {
    const void* pod_this = reinterpret_cast<const void*>(this + 1);
    const void* pod_other = reinterpret_cast<const void*>(other + 1);
    return (DrawAtlasBaseOp::equals(other, pod_this, pod_other))
               ? DisplayListCompare::kEqual
               : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(DrawAtlasOp) == 24);

// Has array data following the struct as per DrawAtlasBaseOp
struct DrawAtlasCulledOp final : DrawAtlasBaseOp {
  static constexpr auto kType = DisplayListOpType::kDrawAtlasCulled;

  DrawAtlasCulledOp(const sk_sp<DlImage>& atlas,
                    int count,
                    DlBlendMode mode,
                    DlImageSampling sampling,
                    bool has_colors,
                    const SkRect& cull_rect,
                    bool render_with_attributes)
      : DrawAtlasBaseOp(atlas,
                        count,
                        mode,
                        sampling,
                        has_colors,
                        render_with_attributes),
        cull_rect(cull_rect) {}

  const SkRect cull_rect;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      const SkRSXform* xform = reinterpret_cast<const SkRSXform*>(this + 1);
      const SkRect* tex = reinterpret_cast<const SkRect*>(xform + count);
      const DlColor* colors =
          has_colors ? reinterpret_cast<const DlColor*>(tex + count) : nullptr;
      const DlBlendMode mode = static_cast<DlBlendMode>(mode_index);
      ctx.receiver.drawAtlas(atlas, xform, tex, colors, count, mode, sampling,
                             &cull_rect, render_with_attributes);
    }
  }

  DisplayListCompare equals(const DrawAtlasCulledOp* other) const {
    const void* pod_this = reinterpret_cast<const void*>(this + 1);
    const void* pod_other = reinterpret_cast<const void*>(other + 1);
    return (cull_rect == other->cull_rect &&
            DrawAtlasBaseOp::equals(other, pod_this, pod_other))
               ? DisplayListCompare::kEqual
               : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(DrawAtlasCulledOp) == 40);

struct DrawDisplayListOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawDisplayList;

  explicit DrawDisplayListOp(const sk_sp<DisplayList>& display_list,
                             SkScalar opacity)
      : opacity(opacity), display_list(display_list) {}

  SkScalar opacity;
  const sk_sp<DisplayList> display_list;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawDisplayList(display_list, opacity);
    }
  }

  DisplayListCompare equals(const DrawDisplayListOp* other) const {
    return (opacity == other->opacity &&
            display_list->Equals(other->display_list))
               ? DisplayListCompare::kEqual
               : DisplayListCompare::kNotEqual;
  }
};
static_assert(sizeof(DrawDisplayListOp) == 16);

// 4 byte header + 8 payload bytes + an aligned pointer take 24 bytes
// (4 unused to align the pointer)
struct DrawTextBlobOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawTextBlob;

  DrawTextBlobOp(const sk_sp<SkTextBlob>& blob, SkScalar x, SkScalar y)
      : x(x), y(y), blob(blob) {}

  const SkScalar x;
  const SkScalar y;
  const sk_sp<SkTextBlob> blob;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawTextBlob(blob, x, y);
    }
  }
};
static_assert(sizeof(DrawTextBlobOp) == 24);

struct DrawTextFrameOp final : DrawOpBase {
  static constexpr auto kType = DisplayListOpType::kDrawTextFrame;

  DrawTextFrameOp(const std::shared_ptr<impeller::TextFrame>& text_frame,
                  SkScalar x,
                  SkScalar y)
      : x(x), y(y), text_frame(text_frame) {}

  const SkScalar x;
  const SkScalar y;
  const std::shared_ptr<impeller::TextFrame> text_frame;

  void dispatch(DispatchContext& ctx) const {
    if (op_needed(ctx)) {
      ctx.receiver.drawTextFrame(text_frame, x, y);
    }
  }
};
static_assert(sizeof(DrawTextFrameOp) == 32);

#define DEFINE_DRAW_SHADOW_OP(name, transparent_occluder)                    \
  struct Draw##name##Op final : DrawOpBase {                                 \
    static constexpr auto kType = DisplayListOpType::kDraw##name;            \
                                                                             \
    Draw##name##Op(const SkPath& path,                                       \
                   DlColor color,                                            \
                   SkScalar elevation,                                       \
                   SkScalar dpr)                                             \
        : color(color), elevation(elevation), dpr(dpr), cached_path(path) {} \
                                                                             \
    const DlColor color;                                                     \
    const SkScalar elevation;                                                \
    const SkScalar dpr;                                                      \
    const DlOpReceiver::CacheablePath cached_path;                           \
                                                                             \
    void dispatch(DispatchContext& ctx) const {                              \
      if (op_needed(ctx)) {                                                  \
        if (ctx.receiver.PrefersImpellerPaths()) {                           \
          ctx.receiver.drawShadow(cached_path, color, elevation,             \
                                  transparent_occluder, dpr);                \
        } else {                                                             \
          ctx.receiver.drawShadow(cached_path.sk_path, color, elevation,     \
                                  transparent_occluder, dpr);                \
        }                                                                    \
      }                                                                      \
    }                                                                        \
                                                                             \
    DisplayListCompare equals(const Draw##name##Op* other) const {           \
      return color == other->color && elevation == other->elevation &&       \
                     dpr == other->dpr && cached_path == other->cached_path  \
                 ? DisplayListCompare::kEqual                                \
                 : DisplayListCompare::kNotEqual;                            \
    }                                                                        \
  };
DEFINE_DRAW_SHADOW_OP(Shadow, false)
DEFINE_DRAW_SHADOW_OP(ShadowTransparentOccluder, true)
#undef DEFINE_DRAW_SHADOW_OP
static_assert(sizeof(DrawShadowOp) == 48);

#pragma pack(pop, DLOpPackLabel)

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DL_OP_RECORDS_H_
