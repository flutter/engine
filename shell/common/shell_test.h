// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_SHELL_TEST_H_
#define FLUTTER_SHELL_COMMON_SHELL_TEST_H_

#include <memory>

#include "flutter/common/settings.h"
#include "flutter/fml/macros.h"
#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/shell/common/run_configuration.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "flutter/testing/test_dart_native_resolver.h"
#include "flutter/testing/test_gl_surface.h"
#include "flutter/testing/thread_test.h"

namespace flutter {
namespace testing {

class ShellTest : public ThreadTest {
 public:
  ShellTest();

  ~ShellTest();

  Settings CreateSettingsForFixture();
  std::unique_ptr<Shell> CreateShell(Settings settings);
  std::unique_ptr<Shell> CreateShell(Settings settings,
                                     TaskRunners task_runners);

  // Simulate n input events where the i-th one is delivered at
  // delivery_time(i).
  //
  // Simulation results will be written into events_consumed_at_frame whose
  // length will be equal to the number of frames drawn. Each element in the
  // vector is the number of input events consumed in that frame. (We can't
  // return such vector because ASSERT_TRUE requires return type of void.)
  //
  // We assume (and check) that the delivery latency is some base latency plus a
  // random latency where the random latency must be within one frame:
  //
  // 1. latency = delivery_time(i) - j * frame_time = base_latency +
  //    random_latency
  // 2. 0 <= base_latency, 0 <= random_latency < frame_time
  //
  // We also assume that there will be at least one input event per frame if
  // there were no latency. Let j = floor( (delivery_time(i) - base_latency) /
  // frame_time ) be the frame index if there were no latency. Then the set of j
  // should be all integers from 0 to continuous_frame_count - 1 for some
  // integer continuous_frame_count.
  //
  // (Note that there coulds be multiple input events within one frame.)
  //
  // The test here is insensitive to the choice of time unit as long as
  // delivery_time and frame_time are in the same unit.
  void TestSimulatedInputEvents(int num_events,
                                int base_latency,
                                std::function<int(int)> delivery_time,
                                int frame_time,
                                std::vector<int>& events_consumed_at_frame);

  TaskRunners GetTaskRunnersForFixture();

  void SendEnginePlatformMessage(Shell* shell,
                                 fml::RefPtr<PlatformMessage> message);

  void AddNativeCallback(std::string name, Dart_NativeFunction callback);

  static void PlatformViewNotifyCreated(
      Shell* shell);  // This creates the surface
  static void RunEngine(Shell* shell, RunConfiguration configuration);

  static void PumpOneFrame(Shell* shell);
  static void DispatchFakePointerData(Shell* shell);

  // Declare |UnreportedTimingsCount|, |GetNeedsReportTimings| and
  // |SetNeedsReportTimings| inside |ShellTest| mainly for easier friend class
  // declarations as shell unit tests and Shell are in different name spaces.

  static bool GetNeedsReportTimings(Shell* shell);
  static void SetNeedsReportTimings(Shell* shell, bool value);

  // Do not assert |UnreportedTimingsCount| to be positive in any tests.
  // Otherwise those tests will be flaky as the clearing of unreported timings
  // is unpredictive.
  static int UnreportedTimingsCount(Shell* shell);

 protected:
  // |testing::ThreadTest|
  void SetUp() override;

  // |testing::ThreadTest|
  void TearDown() override;

 private:
  fml::UniqueFD assets_dir_;
  std::shared_ptr<TestDartNativeResolver> native_resolver_;
  std::unique_ptr<ThreadHost> thread_host_;

  void SetSnapshotsAndAssets(Settings& settings);
};

class ShellTestPlatformView : public PlatformView, public GPUSurfaceGLDelegate {
 public:
  ShellTestPlatformView(PlatformView::Delegate& delegate,
                        TaskRunners task_runners);

  ~ShellTestPlatformView() override;

 private:
  TestGLSurface gl_surface_;

  // |PlatformView|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  // |PlatformView|
  std::unique_ptr<PointerDataDispatcher> MakePointerDataDispatcher(
      Animator& animator,
      RuntimeController& controller) override;

  // |GPUSurfaceGLDelegate|
  bool GLContextMakeCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextClearCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextPresent() override;

  // |GPUSurfaceGLDelegate|
  intptr_t GLContextFBO() const override;

  // |GPUSurfaceGLDelegate|
  GLProcResolver GetGLProcResolver() const override;

  // |GPUSurfaceGLDelegate|
  ExternalViewEmbedder* GetExternalViewEmbedder() override;

  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestPlatformView);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_SHELL_TEST_H_
