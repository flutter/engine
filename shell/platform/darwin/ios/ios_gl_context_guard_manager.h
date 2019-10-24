// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ios_gl_guard_manager_h
#define ios_gl_guard_manager_h

#define GLES_SILENCE_DEPRECATION

#import <OpenGLES/EAGL.h>
#include <map>
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/flow/gl_context_guard_manager.h"

namespace flutter {

class IOSGLContextGuardManager final : public GLContextGuardManager {

public:

  IOSGLContextGuardManager(fml::scoped_nsobject<EAGLContext> flutter_gl_context):flutter_gl_context_(flutter_gl_context){
    stored_ = fml::scoped_nsobject<NSMutableArray> ([[NSMutableArray new] retain]);
  };
  
  ~IOSGLContextGuardManager() = default;

  void SetOtherContextToCurrent() override;

  void SaveOtherContext() override;

  void SetFlutterContextToCurrent() override;

 private:
  fml::scoped_nsobject<EAGLContext> flutter_gl_context_;

  fml::scoped_nsobject<NSMutableArray> stored_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSGLContextGuardManager);
};

}

#endif
