// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_H_

// Should define one and only one of the following
#undef DISPLAY_LIST_USES_SKIA_ALIASES
#define DISPLAY_LIST_USES_SKIA_ADAPTERS
#undef DISPLAY_LIST_USES_IMPELLER_ADAPTERS
#undef DISPLAY_LIST_USES_IMPELLER_ALIASES  // eventual final state - TBD

#if defined(DISPLAY_LIST_USES_SKIA_ALIASES)
#include "flutter/display_list/geometry/dl_geometry_types_skia_aliases.h"
#elif defined(DISPLAY_LIST_USES_SKIA_ADAPTERS)
#include "flutter/display_list/geometry/dl_geometry_types_skia_adapters.h"
#elif defined(DISPLAY_LIST_USES_IMPELLER_ADAPTERS)
#include "flutter/display_list/geometry/dl_geometry_types_impeller_adapters.h"
#endif  // DISPLAY_LIST_USES_*

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_GEOMETRY_TYPES_H_
