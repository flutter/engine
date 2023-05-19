#include <memory>

#include "flutter/impeller/renderer/backend/vulkan/context_vk.h"
#include "flutter/shell/platform/android/android_context_vulkan_impeller.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(AndroidContextVulkanImpeller, DoesNotCreateOwnMessageLoop) {
  auto loop = fml::ConcurrentMessageLoop::Create();
  auto context = std::make_unique<AndroidContextVulkanImpeller>(
      false, loop->GetTaskRunner());
  ASSERT_TRUE(context);

  auto impeller_context = std::static_pointer_cast<impeller::ContextVK>(
      context->GetImpellerContext());
  ASSERT_TRUE(impeller_context);

  impeller_context->GetConcurrentWorkerTaskRunner()->PostTask(
      [loop]() { ASSERT_TRUE(loop->RunsTasksOnCurrentThread()); });
}

}  // namespace testing
}  // namespace flutter
