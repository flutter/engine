#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <cstdint>
#include <deque>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "flutter/shell/platform/android/android_context_gl_skia.h"

namespace flutter {

class AndroidGLContextSwitch {
 public:
  explicit AndroidGLContextSwitch(EGLContext ctx_to_switch_to) {
    old_context_ = eglGetCurrentContext();
    MakeCurrent(ctx_to_switch_to);
  }

  ~AndroidGLContextSwitch() { MakeCurrent(old_context_); }

 private:
  void MakeCurrent(EGLContext context) {
    EGLDisplay display = eglGetCurrentDisplay();
    EGLSurface draw_surface = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface read_surface = eglGetCurrentSurface(EGL_READ);
    EGLBoolean result =
        eglMakeCurrent(display, draw_surface, read_surface, context);
    FML_DCHECK(result == EGL_TRUE)
        << "AndroidGLContextSwitch: failed to make context current";
  }

  EGLContext old_context_;
};

struct _FBOPacket {
  uint32_t fbo_id;
  uint32_t texture_id;
};

class AndroidGLFBOPool {
 public:
  explicit AndroidGLFBOPool(AndroidContextGLSkia* android_context_gl_skia) {
    FML_CHECK(android_context_gl_skia)
        << "AndroidGLFBOPool: gl context was null";

    // this has to be a shared context.
    egl_context_ = android_context_gl_skia->CreateNewSharedContext();
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

    FML_DLOG(ERROR) << __PRETTY_FUNCTION__;

    // TODO evaluate if this is OK.
    // AndroidGLContextSwitch ctx_switch(egl_context_);

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

    FML_DLOG(ERROR) << __PRETTY_FUNCTION__;
    AndroidGLContextSwitch ctx_switch(egl_context_);

    // TODO blit to on_screen_fbo 0.
    FML_DLOG(ERROR) << "blit to screen fbo: " << fbo_id;

    GLuint new_fbo_id = 0;
    {
      glGenFramebuffers(1, &new_fbo_id);
      glBindFramebuffer(GL_FRAMEBUFFER, new_fbo_id);
      _FBOPacket packet = fbo_to_packet[fbo_id];
      glBindTexture(GL_TEXTURE_2D, packet.texture_id);
      glBindTexture(GL_TEXTURE_2D, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_2D, packet.texture_id, 0);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);           // draw frame buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, new_fbo_id);  // read frame buffer

    GLFrameInfo frame_info = frame_info_map[fbo_id];
    const auto w = frame_info.width;
    const auto h = frame_info.height;
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    FML_DLOG(ERROR) << "GL error after blit: " << glGetError();

    EGLDisplay display = eglGetCurrentDisplay();
    EGLSurface draw_surface = eglGetCurrentSurface(EGL_DRAW);
    eglSwapBuffers(display, draw_surface);

    unused_fbo_ids.push_back(fbo_id);
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

      _FBOPacket packet;
      packet.fbo_id = fbo_id;
      packet.texture_id = texture;
      fbo_to_packet[fbo_id] = packet;

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
    frame_info_map[fbo_id] = frame_info;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo_id;
  }

  uint32_t CreateTexture(const GLFrameInfo& frame_info) {
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

  std::deque<uint32_t> unused_fbo_ids;
  std::map<uint32_t, _FBOPacket> fbo_to_packet;
  std::map<uint32_t, GLFrameInfo> frame_info_map;

  EGLContext egl_context_;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidGLFBOPool);
};

}  // namespace flutter
