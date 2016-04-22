// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERVIEW_H_
#define FLUTTER_FLUTTERVIEW_H_

#include "sky/services/semantics/semantics.mojom.h"

#include <UIKit/UIKit.h>

@interface FlutterView : UIView

- (void)setSemanticsServer:(semantics::SemanticsServerPtr)semanticsServer;

@end

#endif  // FLUTTER_FLUTTERVIEW_H_
