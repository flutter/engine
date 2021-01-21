// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>

#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/tests/embedder_config_builder.h"
#include "flutter/shell/platform/embedder/tests/embedder_test.h"
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "flutter/testing/testing.h"

#ifdef _WIN32
// winbase.h defines GetCurrentTime as a macro.
#undef GetCurrentTime
#endif

namespace flutter {
namespace testing {

FlutterKeyEventKind unserializeKind(uint64_t kindInt) {
  switch(kindInt) {
    case 1:
      return kFlutterKeyEventKindUp;
    case 2:
      return kFlutterKeyEventKindDown;
    case 3:
      return kFlutterKeyEventKindRepeat;
    default:
      FML_UNREACHABLE();
      return kFlutterKeyEventKindUp;
  }
}

TEST_F(EmbedderTest, CorrectlySerializeKeyData) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();
  char echoed_string[2] = "m"; // Dummy string that holds one char.
  FlutterKeyEvent echoed_event;

  auto native_echo_event = [&](Dart_NativeArguments args) {
    echoed_event.kind = unserializeKind(
        tonic::DartConverter<uint64_t>::FromDart(
            Dart_GetNativeArgument(args, 1)));
    echoed_event.timestamp = tonic::DartConverter<uint64_t>::FromDart(
          Dart_GetNativeArgument(args, 2));
    echoed_event.physical = tonic::DartConverter<uint64_t>::FromDart(
          Dart_GetNativeArgument(args, 3));
    echoed_event.logical = tonic::DartConverter<uint64_t>::FromDart(
          Dart_GetNativeArgument(args, 4));
    echoed_string[0] = (char)tonic::DartConverter<uint64_t>::FromDart(
          Dart_GetNativeArgument(args, 5));
    echoed_event.synthesized = tonic::DartConverter<bool>::FromDart(
          Dart_GetNativeArgument(args, 6));

    message_latch->Signal();
  };

  auto& context = GetEmbedderContext(EmbedderTestContextType::kOpenGLContext);
  context.AddNativeCallback("EchoKeyEvent", CREATE_NATIVE_ENTRY(native_echo_event));

  // fml::AutoResetWaitableEvent latch;
  // context.AddNativeCallback(
  //     "SignalNativeTest", CREATE_NATIVE_ENTRY(([&latch](Dart_NativeArguments) {
  //     })));

  EmbedderConfigBuilder builder(context);
  builder.SetSoftwareRendererConfig();
  builder.SetDartEntrypoint("key_data_echo");

  auto engine = builder.LaunchEngine();
  ASSERT_TRUE(engine.is_valid());

  const FlutterKeyEvent downEventUpperA {
    .struct_size = sizeof(FlutterKeyEvent),
    .timestamp = 1,
    .kind = kFlutterKeyEventKindDown,
    .physical = 0x00070004,
    .logical = 0x00000000061,
    .character = "A",
    .synthesized = false,
  };
  FlutterEngineSendKeyEvent(engine.get(), &downEventUpperA,
      [](bool handled, void* user_data){
      },
      nullptr);
  message_latch->Wait();

  EXPECT_EQ(echoed_event.timestamp, downEventUpperA.timestamp);
  EXPECT_EQ(echoed_event.kind, downEventUpperA.kind);
  EXPECT_EQ(echoed_event.physical, downEventUpperA.physical);
  EXPECT_EQ(echoed_event.logical, downEventUpperA.logical);
  EXPECT_STREQ(echoed_event.character, downEventUpperA.character);
  EXPECT_EQ(echoed_event.synthesized, downEventUpperA.synthesized);
}


}  // namespace testing
}  // namespace flutter
