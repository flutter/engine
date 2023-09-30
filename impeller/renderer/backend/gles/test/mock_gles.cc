// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "GLES3/gl3.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/backend/gles/test/mock_gles.h"

namespace impeller {
namespace testing {

// OpenGLES is not thread safe.
//
// This mutex is used to ensure that only one test is using the mock at a time.
static std::mutex g_test_lock;

static std::weak_ptr<MockGLES> g_mock_gles;

// Has friend visibility into MockGLES to record calls.
void recordCall(const char* name) {
  if (auto mock_gles = g_mock_gles.lock()) {
    mock_gles->RecordCall(name);
  }
}

// This is a stub function that does nothing/records nothing.
void glDoNothing() {}

auto const kMockVendor = (unsigned char*)"MockGLES";
auto const kMockVersion = (unsigned char*)"3.0";
auto const kExtensions = std::vector<unsigned char*>{
    (unsigned char*)"GL_KHR_debug"  //
};

const unsigned char* glGetString(GLenum name) {
  switch (name) {
    case GL_VENDOR:
      return kMockVendor;
    case GL_VERSION:
      return kMockVersion;
    case GL_SHADING_LANGUAGE_VERSION:
      return kMockVersion;
    default:
      return (unsigned char*)"";
  }
}

const unsigned char* glGetStringi(GLenum name, GLuint index) {
  switch (name) {
    case GL_EXTENSIONS:
      return kExtensions[index];
    default:
      return (unsigned char*)"";
  }
}

void glGetIntegerv(GLenum name, int* value) {
  switch (name) {
    case GL_NUM_EXTENSIONS: {
      *value = kExtensions.size();
    } break;
    case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
      *value = 8;
      break;
    default:
      *value = 0;
      break;
  }
}

GLenum glGetError() {
  return GL_NO_ERROR;
}

void glPopDebugGroupKHR() {
  recordCall("PopDebugGroupKHR");
}

void glPushDebugGroupKHR(GLenum source,
                         GLuint id,
                         GLsizei length,
                         const GLchar* message) {
  recordCall("PushDebugGroupKHR");
}

std::shared_ptr<MockGLES> MockGLES::Init() {
  auto mock_gles = std::shared_ptr<MockGLES>(new MockGLES());
  g_mock_gles = mock_gles;
  return mock_gles;
}

const ProcTableGLES::Resolver kMockResolver = [](const char* name) {
  if (strcmp(name, "glPopDebugGroupKHR") == 0) {
    return reinterpret_cast<void*>(&glPopDebugGroupKHR);
  } else if (strcmp(name, "glPushDebugGroupKHR") == 0) {
    return reinterpret_cast<void*>(&glPushDebugGroupKHR);
  } else if (strcmp(name, "glGetString") == 0) {
    return reinterpret_cast<void*>(&glGetString);
  } else if (strcmp(name, "glGetStringi") == 0) {
    return reinterpret_cast<void*>(&glGetStringi);
  } else if (strcmp(name, "glGetIntegerv") == 0) {
    return reinterpret_cast<void*>(&glGetIntegerv);
  } else if (strcmp(name, "glGetError") == 0) {
    return reinterpret_cast<void*>(&glGetError);
  } else {
    return reinterpret_cast<void*>(&glDoNothing);
  }
};

MockGLES::MockGLES() : proc_table_(kMockResolver) {
  g_test_lock.lock();
}

MockGLES::~MockGLES() {
  g_test_lock.unlock();
}

}  // namespace testing
}  // namespace impeller
