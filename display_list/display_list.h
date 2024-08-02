// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_H_

#include <memory>
#include <optional>

#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_sampling_options.h"
#include "flutter/display_list/geometry/dl_rtree.h"
#include "flutter/fml/logging.h"

// The Flutter DisplayList mechanism encapsulates a persistent sequence of
// rendering operations.
//
// This file contains the definitions for:
// DisplayList: the base class that holds the information about the
//              sequence of operations and can dispatch them to a DlOpReceiver
// DlOpReceiver: a pure virtual interface which can be implemented to field
//               the requests for purposes such as sending them to an SkCanvas
//               or detecting various rendering optimization scenarios
// DisplayListBuilder: a class for constructing a DisplayList from DlCanvas
//                     method calls and which can act as a DlOpReceiver as well
//
// Other files include various class definitions for dealing with display
// lists, such as:
// skia/dl_sk_*.h: classes to interact between SkCanvas and DisplayList
//                 (SkCanvas->DisplayList adapter and vice versa)
//
// display_list_utils.h: various utility classes to ease implementing
//                       a DlOpReceiver, including NOP implementations of
//                       the attribute, clip, and transform methods,
//                       classes to track attributes, clips, and transforms
//                       and a class to compute the bounds of a DisplayList
//                       Any class implementing DlOpReceiver can inherit from
//                       these utility classes to simplify its creation
//
// The Flutter DisplayList mechanism is used in a similar manner to the Skia
// SkPicture mechanism.
//
// A DisplayList must be created using a DisplayListBuilder using its stateless
// methods inherited from DlCanvas.
//
// A DisplayList can be read back by implementing the DlOpReceiver virtual
// methods (with help from some of the classes in the utils file) and
// passing an instance to the Dispatch() method, or it can be rendered
// to Skia using a DlSkCanvasDispatcher.
//
// The mechanism is inspired by the SkLiteDL class that is not directly
// supported by Skia, but has been recommended as a basis for custom
// display lists for a number of their customers.

namespace flutter {

#define FOR_EACH_DISPLAY_LIST_OP(V) \
  V(SetAntiAlias)                   \
  V(SetInvertColors)                \
                                    \
  V(SetStrokeCap)                   \
  V(SetStrokeJoin)                  \
                                    \
  V(SetStyle)                       \
  V(SetStrokeWidth)                 \
  V(SetStrokeMiter)                 \
                                    \
  V(SetColor)                       \
  V(SetBlendMode)                   \
                                    \
  V(ClearColorFilter)               \
  V(SetPodColorFilter)              \
                                    \
  V(ClearColorSource)               \
  V(SetPodColorSource)              \
  V(SetImageColorSource)            \
  V(SetRuntimeEffectColorSource)    \
                                    \
  V(ClearImageFilter)               \
  V(SetPodImageFilter)              \
  V(SetSharedImageFilter)           \
                                    \
  V(ClearMaskFilter)                \
  V(SetPodMaskFilter)               \
                                    \
  V(Save)                           \
  V(SaveLayer)                      \
  V(SaveLayerBackdrop)              \
  V(Restore)                        \
                                    \
  V(Translate)                      \
  V(Scale)                          \
  V(Rotate)                         \
  V(Skew)                           \
  V(Transform2DAffine)              \
  V(TransformFullPerspective)       \
  V(TransformReset)                 \
                                    \
  V(ClipIntersectRect)              \
  V(ClipIntersectOval)              \
  V(ClipIntersectRRect)             \
  V(ClipIntersectPath)              \
  V(ClipDifferenceRect)             \
  V(ClipDifferenceOval)             \
  V(ClipDifferenceRRect)            \
  V(ClipDifferencePath)             \
                                    \
  V(DrawPaint)                      \
  V(DrawColor)                      \
                                    \
  V(DrawLine)                       \
  V(DrawDashedLine)                 \
  V(DrawRect)                       \
  V(DrawOval)                       \
  V(DrawCircle)                     \
  V(DrawRRect)                      \
  V(DrawDRRect)                     \
  V(DrawArc)                        \
  V(DrawPath)                       \
                                    \
  V(DrawPoints)                     \
  V(DrawLines)                      \
  V(DrawPolygon)                    \
  V(DrawVertices)                   \
                                    \
  V(DrawImage)                      \
  V(DrawImageWithAttr)              \
  V(DrawImageRect)                  \
  V(DrawImageNine)                  \
  V(DrawImageNineWithAttr)          \
  V(DrawAtlas)                      \
  V(DrawAtlasCulled)                \
                                    \
  V(DrawDisplayList)                \
  V(DrawTextBlob)                   \
  V(DrawTextFrame)                  \
                                    \
  V(DrawShadow)                     \
  V(DrawShadowTransparentOccluder)

#define DL_OP_TO_ENUM_VALUE(name) k##name,
enum class DisplayListOpType {
  FOR_EACH_DISPLAY_LIST_OP(DL_OP_TO_ENUM_VALUE)
#ifdef IMPELLER_ENABLE_3D
      DL_OP_TO_ENUM_VALUE(SetSceneColorSource)
#endif  // IMPELLER_ENABLE_3D
};
#undef DL_OP_TO_ENUM_VALUE

class DlOpReceiver;
class DisplayListBuilder;

class SaveLayerOptions {
 public:
  static const SaveLayerOptions kWithAttributes;
  static const SaveLayerOptions kNoAttributes;

  SaveLayerOptions() : flags_(0) {}
  SaveLayerOptions(const SaveLayerOptions& options) : flags_(options.flags_) {}
  explicit SaveLayerOptions(const SaveLayerOptions* options)
      : flags_(options->flags_) {}

  SaveLayerOptions without_optimizations() const {
    SaveLayerOptions options;
    options.fRendersWithAttributes = fRendersWithAttributes;
    options.fBoundsFromCaller = fBoundsFromCaller;
    return options;
  }

  bool renders_with_attributes() const { return fRendersWithAttributes; }
  SaveLayerOptions with_renders_with_attributes() const {
    SaveLayerOptions options(this);
    options.fRendersWithAttributes = true;
    return options;
  }

  bool can_distribute_opacity() const { return fCanDistributeOpacity; }
  SaveLayerOptions with_can_distribute_opacity() const {
    SaveLayerOptions options(this);
    options.fCanDistributeOpacity = true;
    return options;
  }

  // Returns true iff the bounds for the saveLayer operation were provided
  // by the caller, otherwise the bounds will have been computed by the
  // DisplayListBuilder and provided for reference.
  bool bounds_from_caller() const { return fBoundsFromCaller; }
  SaveLayerOptions with_bounds_from_caller() const {
    SaveLayerOptions options(this);
    options.fBoundsFromCaller = true;
    return options;
  }
  SaveLayerOptions without_bounds_from_caller() const {
    SaveLayerOptions options(this);
    options.fBoundsFromCaller = false;
    return options;
  }
  bool bounds_were_calculated() const { return !fBoundsFromCaller; }

  // Returns true iff the bounds for the saveLayer do not fully cover the
  // contained rendering operations. This will only occur if the original
  // caller supplied bounds and those bounds were not a strict superset
  // of the content bounds computed by the DisplayListBuilder.
  bool content_is_clipped() const { return fContentIsClipped; }
  SaveLayerOptions with_content_is_clipped() const {
    SaveLayerOptions options(this);
    options.fContentIsClipped = true;
    return options;
  }

  bool contains_backdrop_filter() const { return fHasBackdropFilter; }
  SaveLayerOptions with_contains_backdrop_filter() const {
    SaveLayerOptions options(this);
    options.fHasBackdropFilter = true;
    return options;
  }

  bool content_is_unbounded() const { return fContentIsUnbounded; }
  SaveLayerOptions with_content_is_unbounded() const {
    SaveLayerOptions options(this);
    options.fContentIsUnbounded = true;
    return options;
  }

  SaveLayerOptions& operator=(const SaveLayerOptions& other) {
    flags_ = other.flags_;
    return *this;
  }
  bool operator==(const SaveLayerOptions& other) const {
    return flags_ == other.flags_;
  }
  bool operator!=(const SaveLayerOptions& other) const {
    return flags_ != other.flags_;
  }

 private:
  union {
    struct {
      unsigned fRendersWithAttributes : 1;
      unsigned fCanDistributeOpacity : 1;
      unsigned fBoundsFromCaller : 1;
      unsigned fContentIsClipped : 1;
      unsigned fHasBackdropFilter : 1;
      unsigned fContentIsUnbounded : 1;
    };
    uint32_t flags_;
  };
};

// Manages a buffer allocated with malloc.
class DisplayListStorage {
 public:
  DisplayListStorage() = default;
  DisplayListStorage(DisplayListStorage&&) = default;

  uint8_t* get() { return ptr_.get(); }

  const uint8_t* get() const { return ptr_.get(); }

  void realloc(size_t count) {
    ptr_.reset(static_cast<uint8_t*>(std::realloc(ptr_.release(), count)));
    FML_CHECK(ptr_);
  }

 private:
  struct FreeDeleter {
    void operator()(uint8_t* p) { std::free(p); }
  };
  std::unique_ptr<uint8_t, FreeDeleter> ptr_;
};

class Culler;

// The base class that contains a sequence of rendering operations
// for dispatch to a DlOpReceiver. These objects must be instantiated
// through an instance of DisplayListBuilder::build().
class DisplayList : public SkRefCnt {
 public:
  DisplayList();

  ~DisplayList();

  void Dispatch(DlOpReceiver& ctx) const;
  void Dispatch(DlOpReceiver& ctx, const SkRect& cull_rect) const;
  void Dispatch(DlOpReceiver& ctx, const SkIRect& cull_rect) const;

  // From historical behavior, SkPicture always included nested bytes,
  // but nested ops are only included if requested. The defaults used
  // here for these accessors follow that pattern.
  size_t bytes(bool nested = true) const {
    return sizeof(DisplayList) + byte_count_ +
           (nested ? nested_byte_count_ : 0);
  }

  uint32_t op_count(bool nested = false) const {
    return op_count_ + (nested ? nested_op_count_ : 0);
  }

  uint32_t total_depth() const { return total_depth_; }

  uint32_t unique_id() const { return unique_id_; }

  const SkRect& bounds() const { return bounds_; }

  bool has_rtree() const { return rtree_ != nullptr; }
  sk_sp<const DlRTree> rtree() const { return rtree_; }

  bool Equals(const DisplayList* other) const;
  bool Equals(const DisplayList& other) const { return Equals(&other); }
  bool Equals(const sk_sp<const DisplayList>& other) const {
    return Equals(other.get());
  }

  bool can_apply_group_opacity() const { return can_apply_group_opacity_; }
  bool isUIThreadSafe() const { return is_ui_thread_safe_; }

  /// @brief     Indicates if there are any rendering operations in this
  ///            DisplayList that will modify a surface of transparent black
  ///            pixels.
  ///
  /// This condition can be used to determine whether to create a cleared
  /// surface, render a DisplayList into it, and then composite the
  /// result into a scene. It is not uncommon for code in the engine to
  /// come across such degenerate DisplayList objects when slicing up a
  /// frame between platform views.
  bool modifies_transparent_black() const {
    return modifies_transparent_black_;
  }

  const DisplayListStorage& GetStorage() const { return storage_; }

  /// @brief    Indicates if there are any saveLayer operations at the root
  ///           surface level of the DisplayList that use a backdrop filter.
  ///
  /// This condition can be used to determine what kind of surface to create
  /// for the root layer into which to render the DisplayList as some GPUs
  /// can support surfaces that do or do not support the readback that would
  /// be required for the backdrop filter to do its work.
  bool root_has_backdrop_filter() const { return root_has_backdrop_filter_; }

  /// @brief    Indicates if a rendering operation at the root level of the
  ///           DisplayList had an unbounded result, not otherwise limited by
  ///           a clip operation.
  ///
  /// This condition can occur in a number of situations. The most common
  /// situation is when there is a drawPaint or drawColor rendering
  /// operation which fills out the entire drawable surface unless it is
  /// bounded by a clip. Other situations include an operation rendered
  /// through an ImageFilter that cannot compute the resulting bounds or
  /// when an unclipped backdrop filter is applied by a save layer.
  bool root_is_unbounded() const { return root_is_unbounded_; }

  /// @brief    Indicates the maximum DlBlendMode used on any rendering op
  ///           in the root surface of the DisplayList.
  ///
  /// This condition can be used to determine what kind of surface to create
  /// for the root layer into which to render the DisplayList as some GPUs
  /// can support surfaces that do or do not support the readback that would
  /// be required for the indicated blend mode to do its work.
  DlBlendMode max_root_blend_mode() const { return max_root_blend_mode_; }

  class Dispatcher;

  /// @brief    A class that holds a reference to a specific operation
  ///           recorded within a DisplayList buffer.
  ///
  /// This bookmark must be obtained from a |DispatchTracker| during an
  /// execution of one of the Tracker's Dispatch methods. These objects
  /// are not guaranteed to be thread safe, but they can be used during
  /// or after the Tracker's Dispatch operation concludes. A reference
  /// to the |DisplayList| from which the bookmark comes is maintained so
  /// that their lifetime is not affected by the lifetime of the DisplayList
  /// or the Tracker.
  ///
  /// A Bookmark obtained from a tracker that is not currently in the process
  /// of one of its Dispatch methods will be a NOP and will log a WARNING.
  class Bookmark {
   public:
    /// Dispatches the single |DlOpReceiver| method referred to by this
    /// bookmark and returns true if the bookmark is valid.
    bool Dispatch(DlOpReceiver& receiver) const;

   private:
    Bookmark(sk_sp<DisplayList> display_list, size_t offset)
        : display_list_(std::move(display_list)), offset_(offset) {}

    sk_sp<DisplayList> display_list_;
    const size_t offset_;

    friend class Dispatcher;
  };

  /// @brief    A class that manages a trackable traversal of the operations
  ///           in a |DisplayList| so that |Bookmark|s can be queried during
  ///           the traversal.
  ///
  /// Bookmarks must be obtained from this tracker during the execution of
  /// one of the its Dispatch methods. A tracker can only be used for one
  /// call of any of its Dispatch methods and an attempt to call any of the
  /// Dispatch methods after that will be a NOP and will log a WARNING.
  ///
  /// An example of using a Dispatcher object to save Bookmarks for later
  /// playback:
  ///
  /// {
  ///   class MyBookmarkRecord {
  ///     DisplayList::Bookmark bookmark;
  ///     MyState other_state;
  ///   };
  ///   class MyBookmarkReceiver : public DlOpReceiver {
  ///    public:
  ///     void consume(sk_sp<DisplayList> display_list) {
  ///       dispatcher_ = DisplayList::Dispatcher(display_list);
  ///       dispatcher_.Dispatch(*this);
  ///     }
  ///
  ///     ...
  ///     // Remember transient state in current_state_
  ///     ...
  ///     void drawRect(const SkRect& rect) {
  ///       if (bookmark_needed()) {
  ///         my_records_.emplace_back(dispatcher_.GetBookmark(),
  ///                                  current_state_);
  ///       }
  ///       ...
  ///     }
  ///     ...
  ///
  ///    private:
  ///     MyState current_state_;
  ///     DisplayList::Dispatcher dispatcher_;
  ///     std::vector<MyBookmarkRecord> my_records_;
  ///   };
  ///
  ///   sk_sp<DisplayList> my_dl = ...;
  ///   MyReceiver my_receiver;
  ///   my_receiver.consume(my_dl);
  ///
  ///   ... later ...
  ///
  ///   for (const auto& my_bookmark : my_receiver.GetRecords()) {
  ///     my_other_receiver.SetState(my_bookmark.other_state);
  ///     my_bookmark.bookmark.Dispatch(my_other_receiver);
  ///   }
  /// }
  ///
  /// Note that the bookmark only dispatches a single DlOpReceiver call and
  /// any state that is associated with that call must be remembered along
  /// with the bookmark. Also, if a bookmark is saved to a save or saveLayer
  /// call then that call must be balanced with a restore call, whether by
  /// getting a bookmark for that restore call from the original stream, or
  /// by just calling restore() manually on the receiver the bookmark is
  /// dispatching to, when appropriate.
  class Dispatcher {
   public:
    explicit Dispatcher(sk_sp<DisplayList> display_list)
        : display_list_(std::move(display_list)),
          current_offset_(kInvalidOffset) {}

    Dispatcher(const Dispatcher& copy) = delete;
    Dispatcher(Dispatcher&& move) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;
    Dispatcher& operator=(Dispatcher&&) = delete;

    /// @brief   Dispatch the tracked display list to the supplied receiver
    ///          and return true if the operation is successful (i.e. the
    ///          tracker has not already been used to dispatch the display
    ///          list).
    bool Dispatch(DlOpReceiver& receiver);

    /// @brief   Dispatch the tracked display list to the supplied receiver
    ///          with a supplied culling rectangle and return true if the
    ///          operation is successful (i.e. the tracker has not already
    ///          been used to dispatch the display list).
    bool Dispatch(DlOpReceiver& receiver, const SkRect& cull_rect);

    /// @brief   Dispatch the tracked display list to the supplied receiver
    ///          with a supplied culling rectangle and return true if the
    ///          operation is successful (i.e. the tracker has not already
    ///          been used to dispatch the display list).
    bool Dispatch(DlOpReceiver& receiver, const SkIRect& cull_rect);

    /// @brief   Obtain a bookmark to the operation currently being
    ///          dispatched by the tracker.
    ///
    /// The Bookmark can be used to re-dispatch the current |DlOpReceiver|
    /// call and only that one call with no additional calls to re-establish
    /// the current DisplayList state. As such, it is the responsibility
    /// of the receiver asking for the bookmark to remember any contextual
    /// state for that call, such as previous attribute, transform, and clip
    /// settings.
    Bookmark GetBookmark() const;

   private:
    sk_sp<DisplayList> display_list_;
    size_t current_offset_;

    friend class DisplayList;
  };

 private:
  DisplayList(DisplayListStorage&& ptr,
              size_t byte_count,
              uint32_t op_count,
              size_t nested_byte_count,
              uint32_t nested_op_count,
              uint32_t total_depth,
              const SkRect& bounds,
              bool can_apply_group_opacity,
              bool is_ui_thread_safe,
              bool modifies_transparent_black,
              DlBlendMode max_root_blend_mode,
              bool root_has_backdrop_filter,
              bool root_is_unbounded,
              sk_sp<const DlRTree> rtree);

  static constexpr size_t kInvalidOffset = 1u;
  static constexpr bool IsValidOffset(size_t offset) {
    return (offset & 0x1) == 0;
  }

  static uint32_t next_unique_id();

  static void DisposeOps(const uint8_t* ptr, const uint8_t* end);

  const DisplayListStorage storage_;
  const size_t byte_count_;
  const uint32_t op_count_;

  const size_t nested_byte_count_;
  const uint32_t nested_op_count_;

  const uint32_t total_depth_;

  const uint32_t unique_id_;
  const SkRect bounds_;

  const bool can_apply_group_opacity_;
  const bool is_ui_thread_safe_;
  const bool modifies_transparent_black_;
  const bool root_has_backdrop_filter_;
  const bool root_is_unbounded_;
  const DlBlendMode max_root_blend_mode_;

  const sk_sp<const DlRTree> rtree_;

  void Dispatch(DlOpReceiver& ctx,
                const uint8_t* ptr,
                const uint8_t* end,
                Culler& culler,
                Dispatcher& tracker) const;

  friend class DisplayListBuilder;
  friend class Bookmark;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_H_
