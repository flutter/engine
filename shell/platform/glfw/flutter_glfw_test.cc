// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/flutter_glfw_private.h"

#include "gtest/gtest.h"

TEST(FlutterGlfwTest, GetLocaleStringFromEnvironment) {
  EXPECT_EQ(
      GetLocaleStringFromEnvironmentVariables("sv:de", NULL, NULL, "sv_SE"),
      "sv:de");
  EXPECT_EQ(
      GetLocaleStringFromEnvironmentVariables(NULL, "en_EN", NULL, "sv_SE"),
      "en_EN");
  EXPECT_EQ(
      GetLocaleStringFromEnvironmentVariables(NULL, NULL, "de_DE", "sv_SE"),
      "de_DE");
  EXPECT_EQ(GetLocaleStringFromEnvironmentVariables(NULL, NULL, NULL, "sv_SE"),
            "sv_SE");
  EXPECT_EQ(GetLocaleStringFromEnvironmentVariables(NULL, NULL, NULL, NULL),
            nullptr);
}

TEST(FlutterGlfwTest, ParseLocaleSimple) {
  std::string s = "en";
  std::string language, territory, codeset, modifier;
  ParseLocale(s, &language, &territory, &codeset, &modifier);
  EXPECT_EQ(language, "en");
  EXPECT_EQ(territory, "");
  EXPECT_EQ(codeset, "");
  EXPECT_EQ(modifier, "");
}

TEST(FlutterGlfwTest, ParseLocaleWithTerritory) {
  std::string s = "en_GB";
  std::string language, territory, codeset, modifier;
  ParseLocale(s, &language, &territory, &codeset, &modifier);
  EXPECT_EQ(language, "en");
  EXPECT_EQ(territory, "GB");
  EXPECT_EQ(codeset, "");
  EXPECT_EQ(modifier, "");
}

TEST(FlutterGlfwTest, ParseLocaleWithCodeset) {
  std::string s = "zh_CN.UTF-8";
  std::string language, territory, codeset, modifier;
  ParseLocale(s, &language, &territory, &codeset, &modifier);
  EXPECT_EQ(language, "zh");
  EXPECT_EQ(territory, "CN");
  EXPECT_EQ(codeset, "UTF-8");
  EXPECT_EQ(modifier, "");
}

TEST(FlutterGlfwTest, ParseLocaleFull) {
  std::string s = "en_GB.ISO-8859-1@euro";
  std::string language, territory, codeset, modifier;
  ParseLocale(s, &language, &territory, &codeset, &modifier);
  EXPECT_EQ(language, "en");
  EXPECT_EQ(territory, "GB");
  EXPECT_EQ(codeset, "ISO-8859-1");
  EXPECT_EQ(modifier, "euro");
}

TEST(FlutterGlfwTest, GetLocalesSimple) {
  const char* locale_string = "en_US";
  std::list<std::string> locale_storage;

  std::vector<std::unique_ptr<FlutterLocale>> locales =
      GetLocales(locale_string, locale_storage);

  EXPECT_EQ(locales.size(), 1UL);

  EXPECT_STREQ(locales[0]->language_code, "en");
  EXPECT_STREQ(locales[0]->country_code, "US");
  EXPECT_STREQ(locales[0]->script_code, nullptr);
  EXPECT_STREQ(locales[0]->variant_code, nullptr);
}

TEST(FlutterGlfwTest, GetLocalesFull) {
  const char* locale_string = "en_GB.ISO-8859-1@euro:en_US:sv:zh_CN.UTF-8";
  std::list<std::string> locale_storage;

  std::vector<std::unique_ptr<FlutterLocale>> locales =
      GetLocales(locale_string, locale_storage);

  EXPECT_EQ(locales.size(), 4UL);

  EXPECT_STREQ(locales[0]->language_code, "en");
  EXPECT_STREQ(locales[0]->country_code, "GB");
  EXPECT_STREQ(locales[0]->script_code, "ISO-8859-1");
  EXPECT_STREQ(locales[0]->variant_code, "euro");

  EXPECT_STREQ(locales[1]->language_code, "en");
  EXPECT_STREQ(locales[1]->country_code, "US");
  EXPECT_STREQ(locales[1]->script_code, nullptr);
  EXPECT_STREQ(locales[1]->variant_code, nullptr);

  EXPECT_STREQ(locales[2]->language_code, "sv");
  EXPECT_STREQ(locales[2]->country_code, nullptr);
  EXPECT_STREQ(locales[2]->script_code, nullptr);
  EXPECT_STREQ(locales[2]->variant_code, nullptr);

  EXPECT_STREQ(locales[3]->language_code, "zh");
  EXPECT_STREQ(locales[3]->country_code, "CN");
  EXPECT_STREQ(locales[3]->script_code, "UTF-8");
  EXPECT_STREQ(locales[3]->variant_code, nullptr);
}

TEST(FlutterGlfwTest, GetLocalesEmpty) {
  const char* locale_string = "";
  std::list<std::string> locale_storage;

  std::vector<std::unique_ptr<FlutterLocale>> locales =
      GetLocales(locale_string, locale_storage);

  EXPECT_EQ(locales.size(), 1UL);

  EXPECT_STREQ(locales[0]->language_code, "C");
  EXPECT_STREQ(locales[0]->country_code, nullptr);
  EXPECT_STREQ(locales[0]->script_code, nullptr);
  EXPECT_STREQ(locales[0]->variant_code, nullptr);
}

TEST(FlutterGlfwTest, GetLocalesNull) {
  const char* locale_string;
  std::list<std::string> locale_storage;

  std::vector<std::unique_ptr<FlutterLocale>> locales =
      GetLocales(locale_string, locale_storage);

  EXPECT_EQ(locales.size(), 1UL);

  EXPECT_STREQ(locales[0]->language_code, "C");
  EXPECT_STREQ(locales[0]->country_code, nullptr);
  EXPECT_STREQ(locales[0]->script_code, nullptr);
  EXPECT_STREQ(locales[0]->variant_code, nullptr);
}
