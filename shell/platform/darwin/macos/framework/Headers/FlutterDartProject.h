// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERDARTPROJECT_H_
#define FLUTTER_FLUTTERDARTPROJECT_H_

#import "FlutterBaseDartProject.h"

/**
 * A set of Flutter and Dart assets used by a `FlutterEngine` to initialize execution.
 *
 * TODO(stuartmorgan): Align API with iOS DartProject, and combine.
 */
// This is an empty header and implementation to avoid breaking changes to the public API.
// The original implementation is moved to darwin/common/framework/Headers/FlutterBaseDartProject.h
FLUTTER_DARWIN_EXPORT
@interface FlutterDartProject : FlutterBaseDartProject

@end

#endif  // FLUTTER_FLUTTERDARTPROJECT_H_
