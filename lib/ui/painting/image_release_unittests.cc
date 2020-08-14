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

  fml::AutoResetWaitableEvent message_latch;
  sk_sp<SkImage> current_image_;
  sk_sp<SkPicture> current_picture_;
};

TEST_F(ImageReleaseTest, ImageReleasedAfterFrame) {
  auto nativeCaptureImageAndPicture = [&](Dart_NativeArguments args) {
    CanvasImage* image = GetNativePeer<CanvasImage>(args, 0);
    Picture* picture = GetNativePeer<Picture>(args, 1);
    ASSERT_FALSE(image->image()->unique());
    ASSERT_FALSE(picture->picture()->unique());
    current_image_ = image->image();
    current_picture_ = picture->picture();
  };

  auto nativeDone = [&](Dart_NativeArguments args) { message_latch.Signal(); };

  Settings settings = CreateSettingsForFixture();
  auto task_runner = CreateNewThread();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           task_runner,             // raster
                           task_runner,             // ui
                           task_runner              // io
  );

  AddNativeCallback("CaptureImageAndPicture",
                    CREATE_NATIVE_ENTRY(nativeCaptureImageAndPicture));
  AddNativeCallback("Done", CREATE_NATIVE_ENTRY(nativeDone));

  std::unique_ptr<Shell> shell = CreateShell(std::move(settings), task_runners);

  ASSERT_TRUE(shell->IsSetup());

  SetViewportMetrics(shell.get(), 800, 600);

  shell->GetPlatformView()->NotifyCreated();

  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("pumpImage");

  shell->RunEngine(std::move(configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch.Wait();

  ASSERT_TRUE(current_picture_);
  ASSERT_TRUE(current_image_);

  // AOT modes are fast enough that we get here before the task runner has
  // had a chance to drain. Make sure that if the picture or image are not
  // already unique, at least one idle notification has a chance to process
  // after rasterization has occurred.
  if (!current_picture_->unique() || !current_image_->unique()) {
    task_runner->PostTask([&, engine = shell->GetEngine()]() {
      ASSERT_TRUE(engine);
      engine->NotifyIdle(Dart_TimelineGetMicros() + 10000);
      message_latch.Signal();
    });
    message_latch.Wait();
  }

  EXPECT_TRUE(current_picture_->unique());
  current_picture_.reset();
  EXPECT_TRUE(current_image_->unique());

  shell->GetPlatformView()->NotifyDestroyed();
  DestroyShell(std::move(shell), std::move(task_runners));
}

}  // namespace testing
}  // namespace flutter
