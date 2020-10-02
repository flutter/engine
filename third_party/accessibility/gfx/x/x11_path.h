// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_X_X11_PATH_H_
#define UI_GFX_X_X11_PATH_H_

#include "ui/gfx/gfx_export.h"
#include "ui/gfx/x/xproto.h"

class SkPath;
class SkRegion;

namespace gfx {

// Creates a new XRegion given |region|. The caller is responsible for
// destroying the returned region.
GFX_EXPORT std::unique_ptr<std::vector<x11::Rectangle>>
CreateRegionFromSkRegion(const SkRegion& region);

// Creates a new XRegion given |path|. The caller is responsible for destroying
// the returned region.
GFX_EXPORT std::unique_ptr<std::vector<x11::Rectangle>> CreateRegionFromSkPath(
    const SkPath& path);

}  // namespace gfx

#endif  // UI_GFX_X_X11_PATH_H_
