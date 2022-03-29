// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_VERTICES_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_VERTICES_H_

#include "flutter/display_list/types.h"

namespace flutter {

enum class DlVertexMode {
  kTriangles_VertexMode,
  kTriangleStrip_VertexMode,
  kTriangleFan_VertexMode,
};

inline SkVertices::VertexMode ToSk(DlVertexMode dl_mode) {
  return static_cast<SkVertices::VertexMode>(dl_mode);
}

inline DlVertexMode ToDl(SkVertices::VertexMode sk_mode) {
  return static_cast<DlVertexMode>(sk_mode);
}

class DlVertices {
 public:
  class Builder {
   public:
    union Flags {
      struct {
        unsigned has_texture_coordinates : 1;
        unsigned has_colors : 1;
      };
      uint32_t mask = 0;

      inline Flags operator|(const Flags& rhs) const {
        return {.mask = (mask | rhs.mask)};
      }

      inline Flags& operator|=(const Flags& rhs) {
        mask = mask | rhs.mask;
        return *this;
      }
    };
    static constexpr Flags kNone = {{false, false}};
    static constexpr Flags kHasTextureCoordinates = {{true, false}};
    static constexpr Flags kHasColors = {{false, true}};

    Builder(DlVertexMode mode, int vertex_count, Flags flags, int index_count);

    bool is_valid() { return vertices_ != nullptr; }

    void store_vertices(const SkPoint points[]);
    void store_vertices(const float coordinates[]);
    void store_texture_coordinates(const SkPoint points[]);
    void store_texture_coordinates(const float coordinates[]);
    void store_colors(const SkColor colors[]);
    void store_indices(const uint16_t indices[]);

    std::shared_ptr<DlVertices> build();

   private:
    std::shared_ptr<DlVertices> vertices_;
    bool needs_vertices_;
    bool needs_texture_coords_;
    bool needs_colors_;
    bool needs_indices_;
  };

  static std::shared_ptr<DlVertices> Make(DlVertexMode mode,
                                          int vertex_count,
                                          const SkPoint vertices[],
                                          const SkPoint texture_coordinates[],
                                          const SkColor colors[],
                                          int index_count = 0,
                                          const uint16_t indices[] = nullptr);

  size_t size() const;

  SkRect bounds() const { return bounds_; }

  DlVertexMode mode() const { return mode_; }

  int vertex_count() const { return vertex_count_; }

  const SkPoint* vertices() const {
    return static_cast<const SkPoint*>(pod(vertices_offset_));
  }

  const SkPoint* texture_coordinates() const {
    return static_cast<const SkPoint*>(pod(texture_coordinates_offset_));
  }

  const SkColor* colors() const {
    return static_cast<const SkColor*>(pod(colors_offset_));
  }

  int index_count() const { return index_count_; }

  const uint16_t* indices() const {
    return static_cast<const uint16_t*>(pod(indices_offset_));
  }

  sk_sp<SkVertices> skia_object() const;

  bool operator==(DlVertices const& other) const;

  bool operator!=(DlVertices const& other) const { return !(*this == other); }

 private:
  // Constructors are designed to encapsulate arrays sequentially in memory
  // which means they can only be called by intantiations that use the
  // new (ptr) paradigm which precomputes and preallocates the memory for
  // the class body and all of its arrays, such as in Builder.
  DlVertices(DlVertexMode mode,
             int vertex_count,
             const SkPoint vertices[],
             const SkPoint texture_coordinates[],
             const SkColor colors[],
             int index_count,
             const uint16_t indices[],
             const SkRect* bounds = nullptr);

  // This constructor is specifically used by the DlVertices::Builder to
  // establish the object before the copying of data is requested.
  DlVertices(DlVertexMode mode,
             int vertex_count,
             Builder::Flags flags,
             int index_count);

  // The copy constructor has the same memory pre-allocation requirements
  // as this other constructors. This particular version is used by the
  // DisplaylistBuilder to copy the instance into pre-allocated pod memory
  // in the display list buffer.
  explicit DlVertices(const DlVertices* other);

  DlVertexMode mode_;

  int vertex_count_;
  size_t vertices_offset_;
  size_t texture_coordinates_offset_;
  size_t colors_offset_;

  int index_count_;
  size_t indices_offset_;

  SkRect bounds_;

  const void* pod(int offset) const {
    if (offset <= 0) {
      return nullptr;
    }
    const void* base = static_cast<const void*>(this);
    return static_cast<const char*>(base) + offset;
  }

  friend class DisplayListBuilder;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_VERTICES_H_
