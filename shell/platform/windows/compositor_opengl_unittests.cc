// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "flutter/shell/platform/windows/compositor_opengl.h"
#include "flutter/shell/platform/windows/testing/engine_modifier.h"
#include "flutter/shell/platform/windows/testing/flutter_windows_engine_builder.h"
#include "flutter/shell/platform/windows/testing/windows_test.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

namespace {
using ::testing::Return;

const unsigned char* MockGetString(GLenum name) {
  switch (name) {
    case GL_VERSION:
    case GL_SHADING_LANGUAGE_VERSION:
      return (unsigned char*)"3.0";
    default:
      return (unsigned char*)"";
  }
}

const unsigned char* MockGetStringi(GLenum name, GLuint index) {
  return (unsigned char*)"";
}

void MockGetIntegerv(GLenum name, int* value) {
  *value = 0;
}

GLenum MockGetError() {
  return GL_NO_ERROR;
}

void DoNothing() {}

const impeller::ProcTableGLES::Resolver kMockResolver = [](const char* name) {
  if (strcmp(name, "glGetString") == 0) {
    return reinterpret_cast<void*>(&MockGetString);
  } else if (strcmp(name, "glGetStringi") == 0) {
    return reinterpret_cast<void*>(&MockGetStringi);
  } else if (strcmp(name, "glGetIntegerv") == 0) {
    return reinterpret_cast<void*>(&MockGetIntegerv);
  } else if (strcmp(name, "glGetError") == 0) {
    return reinterpret_cast<void*>(&MockGetError);
  } else {
    return reinterpret_cast<void*>(&DoNothing);
  }
};

class MockAngleSurfaceManager : public AngleSurfaceManager {
 public:
  MockAngleSurfaceManager() : AngleSurfaceManager(false) {}

  MOCK_METHOD(bool, MakeCurrent, (), (override));

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(MockAngleSurfaceManager);
};

class CompositorOpenGLTest : public WindowsTest {};

}  // namespace

TEST_F(CompositorOpenGLTest, CreateBackingStore) {
  auto surface_manager = std::make_unique<MockAngleSurfaceManager>();

  EXPECT_CALL(*surface_manager.get(), MakeCurrent).WillOnce(Return(true));

  auto engine = FlutterWindowsEngineBuilder{GetContext()}.Build();
  EngineModifier modifier(engine.get());
  modifier.SetSurfaceManager(surface_manager.release());

  auto compositor = CompositorOpenGL{engine.get(), kMockResolver};

  FlutterBackingStoreConfig config = {};
  FlutterBackingStore backing_store = {};

  ASSERT_TRUE(compositor.CreateBackingStore(config, &backing_store));
  ASSERT_TRUE(compositor.CollectBackingStore(&backing_store));
}

TEST_F(CompositorOpenGLTest, InitializationFailure) {
  auto surface_manager = std::make_unique<MockAngleSurfaceManager>();

  EXPECT_CALL(*surface_manager.get(), MakeCurrent).WillOnce(Return(false));

  auto engine = FlutterWindowsEngineBuilder{GetContext()}.Build();
  EngineModifier modifier(engine.get());
  modifier.SetSurfaceManager(surface_manager.release());

  auto compositor = CompositorOpenGL{engine.get(), kMockResolver};

  FlutterBackingStoreConfig config = {};
  FlutterBackingStore backing_store = {};

  bool result = compositor.CreateBackingStore(config, &backing_store);

  EXPECT_FALSE(result);
}

}  // namespace testing
}  // namespace flutter
