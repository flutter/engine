#include "flutter/common/task_runners.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/platform/android/context/android_context.h"
#include "flutter/shell/platform/android/jni/jni_mock.h"
#include "flutter/shell/platform/android/platform_view_android.h"
#include "flutter/testing/testing.h"
#include "flutter/testing/thread_test.h"

#include "gmock/gmock.h"  // IWYU pragma: keep
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

namespace {

class MockDelegate final : public ::flutter::PlatformView::Delegate {
 public:
  MOCK_METHOD(void,
              OnPlatformViewCreated,
              (std::unique_ptr<Surface> surface),
              (override));

  MOCK_METHOD(void, OnPlatformViewDestroyed, (), (override));

  MOCK_METHOD(void, OnPlatformViewScheduleFrame, (), (override));

  MOCK_METHOD(void,
              OnPlatformViewSetNextFrameCallback,
              (const fml::closure& closure),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewSetViewportMetrics,
              (int64_t view_id, const ViewportMetrics& metrics),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewDispatchPlatformMessage,
              (std::unique_ptr<PlatformMessage> message),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewDispatchPointerDataPacket,
              (std::unique_ptr<PointerDataPacket> packet),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewDispatchSemanticsAction,
              (int32_t node_id,
               SemanticsAction action,
               fml::MallocMapping args),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewSetSemanticsEnabled,
              (bool enabled),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewSetAccessibilityFeatures,
              (int32_t flags),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewRegisterTexture,
              (std::shared_ptr<Texture> texture),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewUnregisterTexture,
              (int64_t texture_id),
              (override));

  MOCK_METHOD(void,
              OnPlatformViewMarkTextureFrameAvailable,
              (int64_t texture_id),
              (override));

  MOCK_METHOD(void,
              LoadDartDeferredLibrary,
              (intptr_t loading_unit_id,
               std::unique_ptr<const fml::Mapping> snapshot_data,
               std::unique_ptr<const fml::Mapping> snapshot_instructions),
              (override));

  MOCK_METHOD(void,
              LoadDartDeferredLibraryError,
              (intptr_t loading_unit_id,
               const std::string error_message,
               bool transient),
              (override));

  MOCK_METHOD(void,
              UpdateAssetResolverByType,
              (std::unique_ptr<AssetResolver> updated_asset_resolver,
               AssetResolver::AssetResolverType type),
              (override));

  MOCK_METHOD(const Settings&,
              OnPlatformViewGetSettings,
              (),
              (const, override));
};

class AndroidPlatformViewTest : public ThreadTest {
 public:
  MockDelegate& Delegate() { return *delegate_; }

  std::shared_ptr<PlatformViewAndroid> Init(AndroidRenderingAPI api) {
    return std::make_shared<PlatformViewAndroid>(
        Delegate(),
        TaskRunners(/*label=*/GetCurrentTestName(),
                    /*platform=*/GetCurrentTaskRunner(),
                    /*raster=*/CreateNewThread("raster"),
                    /*ui=*/CreateNewThread("ui"),
                    /*io=*/CreateNewThread("io")),
        std::make_shared<JNIMock>(), std::make_shared<AndroidContext>(api));
  }

 private:
  // A mocked delegate to use for testing.
  std::shared_ptr<MockDelegate> delegate_ = std::make_shared<MockDelegate>();
};

}  // namespace

TEST_F(AndroidPlatformViewTest, Create) {
  MockDelegate();
  Init(AndroidRenderingAPI::kVulkan);
}

}  // namespace testing
}  // namespace flutter
