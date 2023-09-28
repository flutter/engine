// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/aiks/canvas.h"
#include "impeller/aiks/canvas_recorder.h"
#include "impeller/aiks/trace_serializer.h"

namespace impeller {

#ifdef IMPELLER_TRACE_CANVAS
using CanvasType = CanvasRecorder<TraceSerializer>;
#else
using CanvasType = Canvas;
#endif

}  // namespace impeller
