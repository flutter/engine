// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GL_CONTEXT_GUARD_MANAGER_H
#define GL_CONTEXT_GUARD_MANAGER_H

namespace flutter {

class GLContextGuardManager {
public:
  class GLGuard {
   public:
    GLGuard(GLContextGuardManager &manager):manager_(manager){
      manager_.RestoreFlutterContext();
    };

    ~GLGuard() {
      manager_.RestoreOtherContext();
    }

  private:
    GLContextGuardManager& manager_;
  };

  virtual void SetOtherContextToCurrent() = 0;

  virtual void SaveOtherContext() = 0;

  virtual void SetFlutterContextToCurrent() = 0;

private:
  void RestoreFlutterContext() {
    SaveOtherContext();
    SetFlutterContextToCurrent();
  }

  void RestoreOtherContext() {
    SetOtherContextToCurrent();
  }
};

}

#endif
