// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_encoding.h"
#include "flutter/lib/ui/painting/image_encoding_impl.h"

#include "flutter/common/task_runners.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/lib/ui/painting/image.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell_test.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::InvokeArgument;
using ::testing::Return;

#if IMPELLER_SUPPORTS_RENDERING
#include "flutter/lib/ui/painting/image_encoding_impeller.h"
#include "impeller/renderer/allocator.h"
#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/context.h"
#include "impeller/renderer/render_target.h"
#include "impeller/renderer/texture.h"
#endif  // IMPELLER_SUPPORTS_RENDERING

// CREATE_NATIVE_ENTRY is leaky by design
// NOLINTBEGIN(clang-analyzer-core.StackAddressEscape)

namespace impeller {
namespace {

class MockDeviceBuffer : public DeviceBuffer {
 public:
  MockDeviceBuffer(const DeviceBufferDescriptor& desc) : DeviceBuffer(desc) {}
  MOCK_METHOD3(CopyHostBuffer,
               bool(const uint8_t* source, Range source_range, size_t offset));

  MOCK_METHOD1(SetLabel, bool(const std::string& label));

  MOCK_METHOD2(SetLabel, bool(const std::string& label, Range range));

  MOCK_CONST_METHOD0(OnGetContents, uint8_t*());

  MOCK_METHOD3(OnCopyHostBuffer,
               bool(const uint8_t* source, Range source_range, size_t offset));
};

class MockAllocator : public Allocator {
 public:
  MOCK_CONST_METHOD0(GetMaxTextureSizeSupported, ISize());
  MOCK_METHOD1(
      OnCreateBuffer,
      std::shared_ptr<DeviceBuffer>(const DeviceBufferDescriptor& desc));
  MOCK_METHOD1(OnCreateTexture,
               std::shared_ptr<Texture>(const TextureDescriptor& desc));
};

class MockBlitPass : public BlitPass {
 public:
  MOCK_CONST_METHOD0(IsValid, bool());
  MOCK_CONST_METHOD1(
      EncodeCommands,
      bool(const std::shared_ptr<Allocator>& transients_allocator));
  MOCK_METHOD1(OnSetLabel, void(std::string label));

  MOCK_METHOD5(OnCopyTextureToTextureCommand,
               bool(std::shared_ptr<Texture> source,
                    std::shared_ptr<Texture> destination,
                    IRect source_region,
                    IPoint destination_origin,
                    std::string label));

  MOCK_METHOD5(OnCopyTextureToBufferCommand,
               bool(std::shared_ptr<Texture> source,
                    std::shared_ptr<DeviceBuffer> destination,
                    IRect source_region,
                    size_t destination_offset,
                    std::string label));

  MOCK_METHOD2(OnGenerateMipmapCommand,
               bool(std::shared_ptr<Texture> texture, std::string label));
};

class MockCommandBuffer : public CommandBuffer {
 public:
  MockCommandBuffer(std::weak_ptr<const Context> context)
      : CommandBuffer(context) {}
  MOCK_CONST_METHOD0(IsValid, bool());
  MOCK_CONST_METHOD1(SetLabel, void(const std::string& label));
  MOCK_CONST_METHOD0(OnCreateBlitPass, std::shared_ptr<BlitPass>());
  MOCK_METHOD1(OnSubmitCommands, bool(CompletionCallback callback));
  MOCK_CONST_METHOD0(OnCreateComputePass, std::shared_ptr<ComputePass>());
  MOCK_METHOD1(OnCreateRenderPass,
               std::shared_ptr<RenderPass>(RenderTarget render_target));
};

class MockImpellerContext : public Context {
 public:
  MOCK_CONST_METHOD0(IsValid, bool());

  MOCK_CONST_METHOD0(GetResourceAllocator, std::shared_ptr<Allocator>());

  MOCK_CONST_METHOD0(GetShaderLibrary, std::shared_ptr<ShaderLibrary>());

  MOCK_CONST_METHOD0(GetSamplerLibrary, std::shared_ptr<SamplerLibrary>());

  MOCK_CONST_METHOD0(GetPipelineLibrary, std::shared_ptr<PipelineLibrary>());

  MOCK_CONST_METHOD0(CreateCommandBuffer, std::shared_ptr<CommandBuffer>());

  MOCK_CONST_METHOD0(GetWorkQueue, std::shared_ptr<WorkQueue>());

  MOCK_CONST_METHOD0(GetGPUTracer, std::shared_ptr<GPUTracer>());

  MOCK_CONST_METHOD0(GetColorAttachmentPixelFormat, PixelFormat());

  MOCK_CONST_METHOD0(GetDeviceCapabilities, const IDeviceCapabilities&());
};

class MockTexture : public Texture {
 public:
  MockTexture(const TextureDescriptor& desc) : Texture(desc) {}
  MOCK_METHOD1(SetLabel, void(std::string_view label));
  MOCK_METHOD3(SetContents,
               bool(const uint8_t* contents, size_t length, size_t slice));
  MOCK_METHOD2(SetContents,
               bool(std::shared_ptr<const fml::Mapping> mapping, size_t slice));
  MOCK_CONST_METHOD0(IsValid, bool());
  MOCK_CONST_METHOD0(GetSize, ISize());
  MOCK_METHOD3(OnSetContents,
               bool(const uint8_t* contents, size_t length, size_t slice));
  MOCK_METHOD2(OnSetContents,
               bool(std::shared_ptr<const fml::Mapping> mapping, size_t slice));
};

}  // namespace
}  // namespace impeller

namespace flutter {
namespace testing {

namespace {
fml::AutoResetWaitableEvent message_latch;

class MockDlImage : public DlImage {
 public:
  MOCK_CONST_METHOD0(skia_image, sk_sp<SkImage>());
  MOCK_CONST_METHOD0(impeller_texture, std::shared_ptr<impeller::Texture>());
  MOCK_CONST_METHOD0(isOpaque, bool());
  MOCK_CONST_METHOD0(isTextureBacked, bool());
  MOCK_CONST_METHOD0(dimensions, SkISize());
  MOCK_CONST_METHOD0(GetApproximateByteSize, size_t());
};

}  // namespace

class MockSyncSwitch {
 public:
  struct Handlers {
    Handlers& SetIfTrue(const std::function<void()>& handler) {
      true_handler = handler;
      return *this;
    }
    Handlers& SetIfFalse(const std::function<void()>& handler) {
      false_handler = handler;
      return *this;
    }
    std::function<void()> true_handler = [] {};
    std::function<void()> false_handler = [] {};
  };

  MOCK_CONST_METHOD1(Execute, void(const Handlers& handlers));
  MOCK_METHOD1(SetSwitch, void(bool value));
};

TEST_F(ShellTest, EncodeImageGivesExternalTypedData) {
  auto native_encode_image = [&](Dart_NativeArguments args) {
    auto image_handle = Dart_GetNativeArgument(args, 0);
    image_handle =
        Dart_GetField(image_handle, Dart_NewStringFromCString("_image"));
    ASSERT_FALSE(Dart_IsError(image_handle)) << Dart_GetError(image_handle);
    ASSERT_FALSE(Dart_IsNull(image_handle));
    auto format_handle = Dart_GetNativeArgument(args, 1);
    auto callback_handle = Dart_GetNativeArgument(args, 2);

    intptr_t peer = 0;
    Dart_Handle result = Dart_GetNativeInstanceField(
        image_handle, tonic::DartWrappable::kPeerIndex, &peer);
    ASSERT_FALSE(Dart_IsError(result));
    CanvasImage* canvas_image = reinterpret_cast<CanvasImage*>(peer);

    int64_t format = -1;
    result = Dart_IntegerToInt64(format_handle, &format);
    ASSERT_FALSE(Dart_IsError(result));

    result = EncodeImage(canvas_image, format, callback_handle);
    ASSERT_TRUE(Dart_IsNull(result));
  };

  auto nativeValidateExternal = [&](Dart_NativeArguments args) {
    auto handle = Dart_GetNativeArgument(args, 0);

    auto typed_data_type = Dart_GetTypeOfExternalTypedData(handle);
    EXPECT_EQ(typed_data_type, Dart_TypedData_kUint8);

    message_latch.Signal();
  };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("EncodeImage", CREATE_NATIVE_ENTRY(native_encode_image));
  AddNativeCallback("ValidateExternal",
                    CREATE_NATIVE_ENTRY(nativeValidateExternal));

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("encodeImageProducesExternalUint8List");

  shell->RunEngine(std::move(configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch.Wait();
  DestroyShell(std::move(shell), task_runners);
}

TEST_F(ShellTest, EncodeImageAccessesSyncSwitch) {
  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  auto native_encode_image = [&](Dart_NativeArguments args) {
    auto image_handle = Dart_GetNativeArgument(args, 0);
    image_handle =
        Dart_GetField(image_handle, Dart_NewStringFromCString("_image"));
    ASSERT_FALSE(Dart_IsError(image_handle)) << Dart_GetError(image_handle);
    ASSERT_FALSE(Dart_IsNull(image_handle));
    auto format_handle = Dart_GetNativeArgument(args, 1);

    intptr_t peer = 0;
    Dart_Handle result = Dart_GetNativeInstanceField(
        image_handle, tonic::DartWrappable::kPeerIndex, &peer);
    ASSERT_FALSE(Dart_IsError(result));
    CanvasImage* canvas_image = reinterpret_cast<CanvasImage*>(peer);

    int64_t format = -1;
    result = Dart_IntegerToInt64(format_handle, &format);
    ASSERT_FALSE(Dart_IsError(result));

    auto io_manager = UIDartState::Current()->GetIOManager();
    fml::AutoResetWaitableEvent latch;

    task_runners.GetIOTaskRunner()->PostTask([&]() {
      auto is_gpu_disabled_sync_switch =
          std::make_shared<const MockSyncSwitch>();
      EXPECT_CALL(*is_gpu_disabled_sync_switch, Execute)
          .WillOnce([](const MockSyncSwitch::Handlers& handlers) {
            handlers.true_handler();
          });
      ConvertToRasterUsingResourceContext(canvas_image->image()->skia_image(),
                                          io_manager->GetResourceContext(),
                                          is_gpu_disabled_sync_switch);
      latch.Signal();
    });

    latch.Wait();

    message_latch.Signal();
  };

  AddNativeCallback("EncodeImage", CREATE_NATIVE_ENTRY(native_encode_image));

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("encodeImageProducesExternalUint8List");

  shell->RunEngine(std::move(configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch.Wait();
  DestroyShell(std::move(shell), task_runners);
}

TEST(ImageEncodingImpellerTest, ConvertDlImageToSkImage) {
  sk_sp<MockDlImage> image(new MockDlImage());
  EXPECT_CALL(*image, dimensions)
      .WillRepeatedly(Return(SkISize::Make(100, 100)));
  impeller::TextureDescriptor desc;
  desc.format = impeller::PixelFormat::kR16G16B16A16Float;
  auto texture = std::make_shared<impeller::MockTexture>(desc);
  EXPECT_CALL(*image, impeller_texture).WillOnce(Return(texture));
  auto context = std::make_shared<impeller::MockImpellerContext>();
  auto command_buffer = std::make_shared<impeller::MockCommandBuffer>(context);
  auto allocator = std::make_shared<impeller::MockAllocator>();
  auto blit_pass = std::make_shared<impeller::MockBlitPass>();
  impeller::DeviceBufferDescriptor device_buffer_desc;
  auto device_buffer =
      std::make_shared<impeller::MockDeviceBuffer>(device_buffer_desc);
  EXPECT_CALL(*allocator, OnCreateBuffer).WillOnce(Return(device_buffer));
  EXPECT_CALL(*blit_pass, IsValid).WillRepeatedly(Return(true));
  EXPECT_CALL(*command_buffer, IsValid).WillRepeatedly(Return(true));
  EXPECT_CALL(*command_buffer, OnCreateBlitPass).WillOnce(Return(blit_pass));
  EXPECT_CALL(*command_buffer, OnSubmitCommands(_))
      .WillOnce(
          DoAll(InvokeArgument<0>(impeller::CommandBuffer::Status::kCompleted),
                Return(true)));
  EXPECT_CALL(*context, GetResourceAllocator).WillRepeatedly(Return(allocator));
  EXPECT_CALL(*context, CreateCommandBuffer).WillOnce(Return(command_buffer));
  bool did_call = false;
  ImageEncodingImpeller::ConvertDlImageToSkImage(
      image,
      [&did_call](sk_sp<SkImage> image) {
        did_call = true;
        EXPECT_TRUE(image);
        EXPECT_EQUALS(100, image->width());
        EXPECT_EQUALS(100, image->height());
      },
      context);
  EXPECT_TRUE(did_call);
}

}  // namespace testing
}  // namespace flutter

// NOLINTEND(clang-analyzer-core.StackAddressEscape)
