#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "flutter/shell/platform/android/android_context_gl_skia.h"
#include "flutter/shell/platform/android/android_egl_surface.h"
#include "fml/task_runner.h"
#include "fml/thread.h"

namespace flutter {

struct _FBOPacket {
  uint32_t fbo_id;
  uint32_t texture_id;
  uint32_t width;
  uint32_t height;
};

class AndroidGLFBOPool {
 public:
  explicit AndroidGLFBOPool(AndroidContextGLSkia* android_context_gl_skia) {
    FML_CHECK(android_context_gl_skia)
        << "AndroidGLFBOPool: gl context was null";

    sb_thread_ = std::make_unique<fml::Thread>("swap_buffers");
    sb_task_runner_ = sb_thread_->GetTaskRunner();

    fml::AutoResetWaitableEvent setup_done;
    sb_task_runner_->PostTask([&, android_context_gl_skia]() {
      // this has to be a shared context, and given that offscreen surfaces
      // are in the same share group as the main gl context, this is fine.
      display_ = android_context_gl_skia->Environment()->Display();
      context_ = android_context_gl_skia->CreateNewSharedContext();

      FML_CHECK(context_ != EGL_NO_CONTEXT);

      const EGLint attribs[] = {
          EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE,
      };
      auto config = android_context_gl_skia->Config();
      surface_ = eglCreatePbufferSurface(display_, config, attribs);

      setup_done.Signal();
    });

    setup_done.Wait();
  }

  ~AndroidGLFBOPool() {
    // TODO evaluate if this is OK.
    // we might have to go back to the main onscreen context here?

    for (auto [k, v] : fbo_to_packet) {
      glDeleteFramebuffers(1, &v.fbo_id);
      glDeleteTextures(1, &v.texture_id);
    }
  }

  uint32_t GetOrCreateFBO(const GLFrameInfo& frame_info) {
    TRACE_EVENT0("flutter", "FBO_POOL_GET_OR_CREATE");
    std::scoped_lock<std::mutex> lock(ds_mutex);

    FML_DLOG(ERROR) << __PRETTY_FUNCTION__;

    if (unused_fbo_ids.empty()) {
      FML_DLOG(ERROR) << "returning a new fbo";
      return CreateFBO(frame_info);
    } else {
      uint32_t fbo_id = unused_fbo_ids[0];
      unused_fbo_ids.pop_front();
      FML_DLOG(ERROR) << "re-using fbo: " << fbo_id;
      return fbo_id;
    }
  }

  void Submit(uint32_t fbo_id) {
    TRACE_EVENT0("flutter", "FBO_POOL_SUBMIT");
    sb_task_runner_->PostTask([&, fbo_id]() {
      // pleaseee work!!
      SubmitInternal(fbo_id);
    });
  }

  void SubmitInternal(uint32_t fbo_id) {
    TRACE_EVENT0("flutter", "FBO_POOL_SUBMIT_INTERNAL");
    eglMakeCurrent(display_, surface_, surface_, context_);

    FML_DLOG(ERROR) << "blit to screen fbo: " << fbo_id;

    _FBOPacket packet;
    {
      std::scoped_lock<std::mutex> lock(ds_mutex);
      packet = fbo_to_packet[fbo_id];
    }

    GLuint new_fbo_id = 0;
    {
      glGenFramebuffers(1, &new_fbo_id);
      glBindFramebuffer(GL_FRAMEBUFFER, new_fbo_id);
      glBindTexture(GL_TEXTURE_2D, packet.texture_id);
      glBindTexture(GL_TEXTURE_2D, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D, packet.texture_id, 0);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (GL_FRAMEBUFFER_COMPLETE != status) {
      FML_DLOG(ERROR) << "[in_submit] fbo failed, status: " << status;
      FML_CHECK(1 < 0);
    } else {
      FML_DLOG(ERROR) << __PRETTY_FUNCTION__ << " FBO_SUCCESS";
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);           // draw frame buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, new_fbo_id);  // read frame buffer

    const auto w = packet.width;
    const auto h = packet.height;
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    FML_DLOG(ERROR) << "GL error after blit: " << glGetError();

    eglSwapBuffers(display_, surface_);
    glFinish();

    {
      std::scoped_lock<std::mutex> lock(ds_mutex);
      unused_fbo_ids.push_back(fbo_id);
    }

    return;
  }

 private:
  uint32_t CreateFBO(const GLFrameInfo& frame_info) {
    GLuint fbo_id = 0;
    glGenFramebuffers(1, &fbo_id);

    {
      // bind 'em all.
      glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
      uint32_t texture = CreateTexture(frame_info);

      fbo_to_packet[fbo_id] = {
          .fbo_id = fbo_id,
          .texture_id = texture,
          .width = frame_info.width,
          .height = frame_info.height,
      };

      glBindTexture(GL_TEXTURE_2D, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D, texture, 0);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (GL_FRAMEBUFFER_COMPLETE != status) {
      FML_DLOG(ERROR) << "fbo failed, status: " << status;
      FML_CHECK(1 < 0);
    } else {
      FML_DLOG(ERROR) << __PRETTY_FUNCTION__ << " FBO_SUCCESS";
    }

    FML_DLOG(ERROR) << "created a new fbo_id: " << fbo_id;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo_id;
  }

  static uint32_t CreateTexture(const GLFrameInfo& frame_info) {
    GLuint texture = 0;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_info.width, frame_info.height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    return texture;
  }

  std::unique_ptr<fml::Thread> sb_thread_;
  fml::RefPtr<fml::TaskRunner> sb_task_runner_;

  std::mutex ds_mutex;
  std::deque<uint32_t> unused_fbo_ids;
  std::map<uint32_t, _FBOPacket> fbo_to_packet;

  // EGL stuff
  EGLContext context_;
  EGLDisplay display_;
  EGLSurface surface_;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidGLFBOPool);
};

}  // namespace flutter
