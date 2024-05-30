// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_WEB_UI_SKWASM_RENDER_STRATEGY_H_
#define FLUTTER_LIB_WEB_UI_SKWASM_RENDER_STRATEGY_H_

#ifdef SKWASM_USE_IMPELLER
#include "impeller/impeller_strategy.h"  // nogncheck
#else
#include "skia/skia_strategy.h"  // nogncheck
#endif

#endif  // FLUTTER_LIB_WEB_UI_SKWASM_RENDER_STRATEGY_H_
