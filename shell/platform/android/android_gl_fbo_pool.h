#pragma once

#include <GLES/gl.h>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"

namespace flutter {

class AndroidGLFBOPool {
 public:
  AndroidGLFBOPool() = default;

  uint32_t GetOrCreateFBO(const GLFrameInfo& frame_info) {
    FML_LOG(ERROR) << __PRETTY_FUNCTION__;
    return 0;
  }

  void Submit(uint32_t fbo_id) {
    FML_LOG(ERROR) << __PRETTY_FUNCTION__;
    return;
  }

 private:
  std::vector<uint32_t> unused_fbo_ids;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidGLFBOPool);
};

}  // namespace flutter
