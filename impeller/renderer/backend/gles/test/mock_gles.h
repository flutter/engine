// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_TEST_MOCK_GLES_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_TEST_MOCK_GLES_H_

#include <memory>
#include <optional>

#include "gmock/gmock.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"

namespace impeller {
namespace testing {

extern const ProcTableGLES::Resolver kMockResolverGLES;

class IMockGLESImpl {
 public:
  virtual ~IMockGLESImpl() = default;
  virtual void DeleteTextures(GLsizei size, const GLuint* queries) {}
  virtual void GenTextures(GLsizei n, GLuint* textures) {}
  virtual void ObjectLabelKHR(GLenum identifier,
                              GLuint name,
                              GLsizei length,
                              const GLchar* label) {}
};

class MockGLESImpl : public IMockGLESImpl {
 public:
  MOCK_METHOD(void,
              DeleteTextures,
              (GLsizei size, const GLuint* queries),
              (override));
  MOCK_METHOD(void, GenTextures, (GLsizei n, GLuint* textures), (override));
  MOCK_METHOD(
      void,
      ObjectLabelKHR,
      (GLenum identifier, GLuint name, GLsizei length, const GLchar* label),
      (override));
};

/// @brief      Provides a mocked version of the |ProcTableGLES| class.
///
/// Typically, Open GLES at runtime will be provided the host's GLES bindings
/// (as function pointers). This class maintains a set of function pointers that
/// appear to be GLES functions, but are actually just stubs that record
/// invocations.
///
/// See `README.md` for more information.
class MockGLES final {
 public:
  static std::shared_ptr<MockGLES> Init(std::unique_ptr<MockGLESImpl> impl);

  /// @brief      Returns an initialized |MockGLES| instance.
  ///
  /// This method overwrites mocked global GLES function pointers to record
  /// invocations on this instance of |MockGLES|. As such, it should only be
  /// called once per test.
  static std::shared_ptr<MockGLES> Init(
      const std::optional<std::vector<const unsigned char*>>& extensions =
          std::nullopt,
      const char* version_string = "OpenGL ES 3.0",
      ProcTableGLES::Resolver resolver = kMockResolverGLES);

  /// @brief      Returns a configured |ProcTableGLES| instance.
  const ProcTableGLES& GetProcTable() const { return proc_table_; }

  /// @brief      Returns a vector of the names of all recorded calls.
  ///
  /// Calls are cleared after this method is called.
  std::vector<std::string> GetCapturedCalls() {
    std::vector<std::string> calls = captured_calls_;
    captured_calls_.clear();
    return calls;
  }

  ~MockGLES();

  IMockGLESImpl* GetImpl() { return impl_.get(); }

 private:
  friend void RecordGLCall(const char* name);
  friend void mockGenTextures(GLsizei n, GLuint* textures);

  explicit MockGLES(ProcTableGLES::Resolver resolver = kMockResolverGLES);

  void RecordCall(const char* name) { captured_calls_.emplace_back(name); }

  ProcTableGLES proc_table_;
  std::vector<std::string> captured_calls_;
  std::unique_ptr<IMockGLESImpl> impl_;

  MockGLES(const MockGLES&) = delete;

  MockGLES& operator=(const MockGLES&) = delete;
};

}  // namespace testing
}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_TEST_MOCK_GLES_H_
