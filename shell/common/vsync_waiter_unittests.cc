#include <initializer_list>

#include "flutter/common/settings.h"
#include "flutter/common/task_runners.h"
#include "flutter/fml/command_line.h"
#include "flutter/shell/common/switches.h"

#include "gtest/gtest.h"
#include "thread_host.h"
#include "vsync_waiter.h"

namespace flutter {
namespace testing {

class TestVsyncWaiter : public VsyncWaiter {
 public:
  explicit TestVsyncWaiter(TaskRunners task_runners)
      : VsyncWaiter(std::move(task_runners)) {}

  int await_vsync_call_count_;

 protected:
  void AwaitVSync() override { await_vsync_call_count_++; }
};

TEST(VsyncWaiterTest, NoUnneededAwaitVsync) {
  using flutter::ThreadHost;
  std::string prefix = "vsync_waiter_test";

  fml::MessageLoop::EnsureInitializedForCurrentThread();
  auto platform_task_runner = fml::MessageLoop::GetCurrent().GetTaskRunner();

  ThreadHost thread_host =
      ThreadHost(prefix, flutter::ThreadHost::Type::RASTER |
                             flutter::ThreadHost::Type::UI |
                             flutter::ThreadHost::Type::IO);
  const flutter::TaskRunners task_runners(
      prefix,                                      // Dart thread labels
      platform_task_runner,                        // platform
      thread_host.raster_thread->GetTaskRunner(),  // raster
      thread_host.ui_thread->GetTaskRunner(),      // ui
      thread_host.io_thread->GetTaskRunner()       // io
  );

  TestVsyncWaiter vsync_waiter(task_runners);

  vsync_waiter.ScheduleSecondaryCallback(1, [] {});
  EXPECT_EQ(vsync_waiter.await_vsync_call_count_, 1);

  vsync_waiter.ScheduleSecondaryCallback(2, [] {});
  EXPECT_EQ(vsync_waiter.await_vsync_call_count_, 1);
}

}  // namespace testing
}  // namespace flutter
