// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/dart_isolate_runner.h"
#include "flutter/testing/fixture_test.h"

#include "tonic/dart_args.h"
#include "tonic/dart_wrappable.h"

namespace flutter {
namespace testing {

class FfiNativeTest : public FixtureTest {
 public:
  FfiNativeTest()
      : settings_(CreateSettingsForFixture()),
        vm_(DartVMRef::Create(settings_)),
        thread_(CreateNewThread()),
        task_runners_(GetCurrentTestName(),
                      thread_,
                      thread_,
                      thread_,
                      thread_) {}

  ~FfiNativeTest() = default;

  [[nodiscard]] bool RunWithEntrypoint(const std::string& entrypoint) {
    if (running_isolate_) {
      return false;
    }
    auto isolate =
        RunDartCodeInIsolate(vm_, settings_, task_runners_, entrypoint, {},
                             GetDefaultKernelFilePath());
    if (!isolate || isolate->get()->GetPhase() != DartIsolate::Phase::Running) {
      return false;
    }

    running_isolate_ = std::move(isolate);
    return true;
  }

 protected:
  Settings settings_;
  DartVMRef vm_;
  std::unique_ptr<AutoIsolateShutdown> running_isolate_;
  fml::RefPtr<fml::TaskRunner> thread_;
  TaskRunners task_runners_;
  FML_DISALLOW_COPY_AND_ASSIGN(FfiNativeTest);
};

fml::AutoResetWaitableEvent event;

void Nop() {
  event.Signal();
}

// Make a simple call through the Tonic FFI bindings.
TEST_F(FfiNativeTest, FfiBindingCallNop) {
  AddFfiNativeCallback(
      "Nop", reinterpret_cast<void*>(
                 tonic::FfiDispatcher<void, decltype(&Nop), &Nop>::Call));

  ASSERT_TRUE(RunWithEntrypoint("nop"));
  event.Wait();

  running_isolate_->Shutdown();
}

Dart_Handle Echo(Dart_Handle str) {
  const char* c_str = nullptr;
  Dart_StringToCString(str, &c_str);
  EXPECT_STREQ(c_str, "Hello World!");
  return str;
}

// Make call with handles through the Tonic FFI bindings.
TEST_F(FfiNativeTest, FfiBindingCallEcho) {
  AddFfiNativeCallback(
      "Echo", reinterpret_cast<void*>(
                  tonic::FfiDispatcher<void, decltype(&Echo), &Echo>::Call));

  AddNativeCallback(
      "SignalDone",
      CREATE_NATIVE_ENTRY([&](Dart_NativeArguments args) { event.Signal(); }));

  ASSERT_TRUE(RunWithEntrypoint("callEcho"));
  event.Wait();

  running_isolate_->Shutdown();
}

void MyTestFunction(intptr_t a, float b) {}

// Serialise a static function without handles.
TEST_F(FfiNativeTest, SerialiseFunction) {
  auto my_test_function_dispatcher =
      tonic::FfiDispatcher<void, decltype(&MyTestFunction), &MyTestFunction>();

  EXPECT_TRUE(my_test_function_dispatcher.AllowedAsLeafCalls());

  EXPECT_STREQ(my_test_function_dispatcher.GetReturnFfiRepresentation(),
               "Void");

  EXPECT_STREQ(my_test_function_dispatcher.GetReturnDartRepresentation(),
               "void");

  {
    std::ostringstream stream_;
    my_test_function_dispatcher.WriteFfiArguments(&stream_);
    if (sizeof(intptr_t) == 8) {
      EXPECT_STREQ(stream_.str().c_str(), "Int64, Float");
    } else {
      ASSERT_TRUE(sizeof(intptr_t) == 4);
      EXPECT_STREQ(stream_.str().c_str(), "Int32, Float");
    }
  }

  {
    std::ostringstream stream_;
    my_test_function_dispatcher.WriteDartArguments(&stream_);
    EXPECT_STREQ(stream_.str().c_str(), "int, double");
  }
}

class MyTestClass : public tonic::DartWrappable {
 public:
  static bool MyTestFunction(tonic::DartWrappable* ptr,
                             double x,
                             Dart_Handle handle) {
    return false;
  }
  Dart_Handle MyTestMethod(bool a) { return nullptr; }
};

// Serialise a static class member function that uses Handles.
TEST_F(FfiNativeTest, SerialiseClassMemberFunction) {
  auto my_test_function_dispatcher =
      tonic::FfiDispatcher<void, decltype(&MyTestClass::MyTestFunction),
                           &MyTestClass::MyTestFunction>();

  EXPECT_FALSE(my_test_function_dispatcher.AllowedAsLeafCalls());

  EXPECT_STREQ(my_test_function_dispatcher.GetReturnFfiRepresentation(),
               "Bool");

  EXPECT_STREQ(my_test_function_dispatcher.GetReturnDartRepresentation(),
               "bool");

  {
    std::ostringstream stream_;
    my_test_function_dispatcher.WriteFfiArguments(&stream_);
    EXPECT_STREQ(stream_.str().c_str(), "Pointer, Double, Handle");
  }

  {
    std::ostringstream stream_;
    my_test_function_dispatcher.WriteDartArguments(&stream_);
    EXPECT_STREQ(stream_.str().c_str(), "Pointer, double, Object");
  }
}

// Serialise an instance method.
TEST_F(FfiNativeTest, SerialiseClassMemberMethod) {
  auto my_test_method_dispatcher =
      tonic::FfiDispatcher<MyTestClass, decltype(&MyTestClass::MyTestMethod),
                           &MyTestClass::MyTestMethod>();

  EXPECT_FALSE(my_test_method_dispatcher.AllowedAsLeafCalls());

  EXPECT_STREQ(my_test_method_dispatcher.GetReturnFfiRepresentation(),
               "Handle");

  EXPECT_STREQ(my_test_method_dispatcher.GetReturnDartRepresentation(),
               "Object");

  {
    std::ostringstream stream_;
    my_test_method_dispatcher.WriteFfiArguments(&stream_);
    EXPECT_STREQ(stream_.str().c_str(), "Pointer, Bool");
  }

  {
    std::ostringstream stream_;
    my_test_method_dispatcher.WriteDartArguments(&stream_);
    EXPECT_STREQ(stream_.str().c_str(), "Pointer, bool");
  }
}
}  // namespace testing
}  // namespace flutter
