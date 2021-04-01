// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/semantics/attributed_string.h"
#include "flutter/shell/common/shell_test.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

using AttributedStringTest = ShellTest;

TEST_F(AttributedStringTest, CanConcatCorrectly) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();

  auto nativeSemanticsUpdate = [message_latch](Dart_NativeArguments args) {
    auto handle = Dart_GetNativeArgument(args, 0);
    intptr_t peer = 0;
    Dart_Handle result = Dart_GetNativeInstanceField(
        handle, tonic::DartWrappable::kPeerIndex, &peer);
    ASSERT_FALSE(Dart_IsError(result));
    AttributedString* attributed_string =
        reinterpret_cast<AttributedString*>(peer);
    ASSERT_EQ(attributed_string->GetString(),
              "stringFragment1stringFragment2stringFragment3");
    ASSERT_EQ(attributed_string->GetAttributes().size(), (size_t)3);
    ASSERT_EQ(attributed_string->GetAttributes()[0]->start, 0);
    ASSERT_EQ(attributed_string->GetAttributes()[0]->end, 1);
    ASSERT_EQ(attributed_string->GetAttributes()[0]->type,
              StringAttributeType::kLocale);
    auto local_attribute = std::static_pointer_cast<LocaleStringAttribute>(
        attributed_string->GetAttributes()[0]);
    ASSERT_EQ(local_attribute->locale, "en-MX");
    ASSERT_EQ(attributed_string->GetAttributes()[1]->start, 18);
    ASSERT_EQ(attributed_string->GetAttributes()[1]->end, 21);
    ASSERT_EQ(attributed_string->GetAttributes()[1]->type,
              StringAttributeType::kSpellOut);
    ASSERT_EQ(attributed_string->GetAttributes()[2]->start, 30);
    ASSERT_EQ(attributed_string->GetAttributes()[2]->end, 36);
    ASSERT_EQ(attributed_string->GetAttributes()[2]->type,
              StringAttributeType::kSpellOut);
    message_latch->Signal();
  };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("SendAttributedString",
                    CREATE_NATIVE_ENTRY(nativeSemanticsUpdate));

  std::unique_ptr<Shell> shell =
      CreateShell(std::move(settings), std::move(task_runners));

  ASSERT_TRUE(shell->IsSetup());
  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("createConcatString");

  shell->RunEngine(std::move(configuration), [](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  DestroyShell(std::move(shell), std::move(task_runners));
}

TEST_F(AttributedStringTest, CanToString) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();

  auto nativeSemanticsUpdate = [message_latch](Dart_NativeArguments args) {
    auto handle = Dart_GetNativeArgument(args, 0);
    std::string to_string_result =
        tonic::DartConverter<std::string>::FromDart(handle);
    ASSERT_EQ(to_string_result,
              "AttributedString('myString', attributes: "
              "[LocaleStringAttribute(TextRange(start: 0, end: 1), en-MX), "
              "SpellOutStringAttribute(TextRange(start: 3, end: 6))])");
    message_latch->Signal();
  };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("SendToStringResult",
                    CREATE_NATIVE_ENTRY(nativeSemanticsUpdate));

  std::unique_ptr<Shell> shell =
      CreateShell(std::move(settings), std::move(task_runners));

  ASSERT_TRUE(shell->IsSetup());
  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("createAttributedString");

  shell->RunEngine(std::move(configuration), [](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  DestroyShell(std::move(shell), std::move(task_runners));
}

}  // namespace testing
}  // namespace flutter
