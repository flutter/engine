// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/common/task_runners.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/lib/ui/painting/picture.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell_test.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"

namespace flutter {
namespace testing {

class ImageReleaseTest : public ShellTest {
 public:
  template <class T>
  T* GetNativePeer(Dart_NativeArguments args, int index) {
    auto handle = Dart_GetNativeArgument(args, index);
    intptr_t peer = 0;
    EXPECT_FALSE(Dart_IsError(Dart_GetNativeInstanceField(
        handle, tonic::DartWrappable::kPeerIndex, &peer)));
    return reinterpret_cast<T*>(peer);
  }

  // Used to wait on Dart callbacks or Shell task runner flushing
  fml::AutoResetWaitableEvent message_latch_;
  // Used to wait for the finalization of the picture Dart_Handle
  fml::AutoResetWaitableEvent picture_finalizer_latch_;
  static void picture_finalizer(void* isolate_callback_data, void* peer) {
    auto latch = reinterpret_cast<fml::AutoResetWaitableEvent*>(peer);
    latch->Signal();
  }
  // Used to wait for the finalization of the image Dart_Handle
  fml::AutoResetWaitableEvent image_finalizer_latch_;
  static void image_finalizer(void* isolate_callback_data, void* peer) {
    auto latch = reinterpret_cast<fml::AutoResetWaitableEvent*>(peer);
    latch->Signal();
  }

  sk_sp<SkPicture> current_picture_;
  sk_sp<SkImage> current_image_;
};

TEST_F(ImageReleaseTest, ImageReleasedAfterFrame) {
  auto native_capture_image_and_picture = [&](Dart_NativeArguments args) {
    CanvasImage* image = GetNativePeer<CanvasImage>(args, 0);
    Picture* picture = GetNativePeer<Picture>(args, 1);
    ASSERT_FALSE(image->image()->unique());
    ASSERT_FALSE(picture->picture()->unique());
    current_image_ = image->image();
    current_picture_ = picture->picture();
    Dart_NewFinalizableHandle(
        Dart_HandleFromWeakPersistent(picture->dart_wrapper()),
        &picture_finalizer_latch_, 0, &picture_finalizer);
    Dart_NewFinalizableHandle(
        Dart_HandleFromWeakPersistent(image->dart_wrapper()),
        &image_finalizer_latch_, 0, &image_finalizer);
  };

  auto native_on_begin_frame_done = [&](Dart_NativeArguments args) {
    message_latch_.Signal();
  };

  Settings settings = CreateSettingsForFixture();
  auto task_runner = CreateNewThread();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           task_runner,             // raster
                           task_runner,             // ui
                           task_runner              // io
  );

  AddNativeCallback("CaptureImageAndPicture",
                    CREATE_NATIVE_ENTRY(native_capture_image_and_picture));
  AddNativeCallback("OnBeginFrameDone",
                    CREATE_NATIVE_ENTRY(native_on_begin_frame_done));

  std::unique_ptr<Shell> shell = CreateShell(std::move(settings), task_runners);

  ASSERT_TRUE(shell->IsSetup());

  SetViewportMetrics(shell.get(), 800, 600);

  shell->GetPlatformView()->NotifyCreated();

  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("pumpImage");

  shell->RunEngine(std::move(configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch_.Wait();

  ASSERT_TRUE(current_picture_);
  ASSERT_TRUE(current_image_);

  // Make sure we get at least one good sized NotifyIdle here.
  task_runner->PostTask([&, engine = shell->GetEngine()]() {
    ASSERT_TRUE(engine);
    engine->NotifyIdle(Dart_TimelineGetMicros() + 10000000);
    message_latch_.Signal();
  });
  message_latch_.Wait();
  picture_finalizer_latch_.Wait();

  EXPECT_TRUE(current_picture_->unique());
  current_picture_.reset();

  image_finalizer_latch_.Wait();
  EXPECT_TRUE(current_image_->unique());
  current_image_.reset();

  shell->GetPlatformView()->NotifyDestroyed();
  DestroyShell(std::move(shell), std::move(task_runners));
}

}  // namespace testing
}  // namespace flutter
