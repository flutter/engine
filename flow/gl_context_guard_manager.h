// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GL_CONTEXT_GUARD_MANAGER_H
#define GL_CONTEXT_GUARD_MANAGER_H

namespace flutter {

// Manages `GLGuard.
//
// Should be subclassed for each platform embedder that uses GL, and requires to
// protect flutter's gl context from other 3rd party librarys, plugins and
// packages.
class GLContextGuardManager {
 public:
  // A `GLGuard` protects the flutter's gl context to be used by other 3rd party
  // librarys, plugins and packages. On construction, it should set flutter's gl
  // context to the current context. On destruction, it should restore the gl
  // context before the construction of this object.
  class GLContextMakeCurrentResult {
   public:
    GLContextMakeCurrentResult() = default;
    
    GLContextMakeCurrentResult(bool make_current_result):
    make_current_result_(make_current_result){}

   bool GetMakeCurrentResult() {return make_current_result_;}

   private:
    bool make_current_result_;
  };
};

}  // namespace flutter

#endif
