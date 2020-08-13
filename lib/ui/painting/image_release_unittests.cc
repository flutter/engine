// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// FLUTTER_NOLINT

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

template <class T>
T* GetNativePeer(Dart_NativeArguments args, int index) {
  auto handle = Dart_GetNativeArgument(args, index);
  intptr_t peer = 0;
  EXPECT_FALSE(Dart_IsError(Dart_GetNativeInstanceField(
      handle, tonic::DartWrappable::kPeerIndex, &peer)));
  return reinterpret_cast<T*>(peer);
}

TEST_F(ShellTest, ImageReleasedAfterFrame) {
  fml::AutoResetWaitableEvent message_latch;

  sk_sp<SkImage> current_image;
  sk_sp<SkPicture> current_picture;
  auto nativeCaptureImageAndPicture = [&](Dart_NativeArguments args) {
    CanvasImage* image = GetNativePeer<CanvasImage>(args, 0);
    Picture* picture = GetNativePeer<Picture>(args, 1);
    ASSERT_FALSE(image->image()->unique());
    ASSERT_FALSE(picture->picture()->unique());
    current_image = image->image();
    current_picture = picture->picture();
  };

  auto nativeDone = [&](Dart_NativeArguments args) { message_latch.Signal(); };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
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

  ASSERT_TRUE(current_picture);
  ASSERT_TRUE(current_image);

  ASSERT_FALSE(current_picture->unique());
  ASSERT_FALSE(current_image->unique());

  // Drain the raster task runner to get to the end of the frame.
  fml::AutoResetWaitableEvent latch;
  task_runners.GetRasterTaskRunner()->PostTask([&latch]() { latch.Signal(); });
  latch.Wait();

  // Tell the engine we're idle.
  task_runners.GetUITaskRunner()->PostTask(
      [&latch, engine = shell->GetEngine()]() {
        ASSERT_TRUE(engine);
        engine->NotifyIdle(Dart_TimelineGetMicros() + 10000);
        latch.Signal();
      });
  latch.Wait();

  EXPECT_TRUE(current_picture->unique());
  current_picture.reset();
  EXPECT_TRUE(current_image->unique());

  shell->GetPlatformView()->NotifyDestroyed();
  DestroyShell(std::move(shell), std::move(task_runners));
}

}  // namespace testing
}  // namespace flutter
