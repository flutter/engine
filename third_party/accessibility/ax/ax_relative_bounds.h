// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_AX_AX_RELATIVE_BOUNDS_H_
#define ACCESSIBILITY_AX_AX_RELATIVE_BOUNDS_H_

#include <stdint.h>

#include <memory>
#include <ostream>

#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkRect.h"

#include "ax_base_export.h"
#include "ax_enums.h"

namespace ax {

// The relative bounding box of an AXNode.
//
// This is an efficient, compact, serializable representation of a node's
// bounding box that requires minimal changes to the tree when layers are
// moved or scrolled. Computing the absolute bounding box of a node requires
// walking up the tree and applying node offsets and transforms until reaching
// the top.
//
// If the offset container id is valid, the bounds are relative
// to the node with that offset container id.
//
// Otherwise, for a node other than the root, the bounds are relative to
// the root of the tree, and for the root of a tree, the bounds are relative
// to its immediate containing node.
struct AX_BASE_EXPORT AXRelativeBounds final {
  AXRelativeBounds();
  virtual ~AXRelativeBounds();

  AXRelativeBounds(const AXRelativeBounds& other);
  AXRelativeBounds& operator=(AXRelativeBounds other);
  bool operator!=(const AXRelativeBounds& other) const;
  bool operator==(const AXRelativeBounds& other) const;

  std::string ToString() const;

  // The id of an ancestor node in the same AXTree that this object's
  // bounding box is relative to, or -1 if there's no offset container.
  int32_t offset_container_id;

  // The relative bounding box of this node.
  SkRect bounds;

  // An additional transform to apply to position this object and its subtree.
  // NOTE: this member is a std::unique_ptr because it's rare and gfx::Transform
  // takes up a fair amount of space. The assignment operator and copy
  // constructor both make a duplicate of the owned pointer, so it acts more
  // like a member than a pointer.
  SkMatrix transform;
};

AX_BASE_EXPORT std::ostream& operator<<(std::ostream& stream,
                                        const AXRelativeBounds& bounds);

}  // namespace ax

#endif  // ACCESSIBILITY_AX_AX_RELATIVE_BOUNDS_H_
