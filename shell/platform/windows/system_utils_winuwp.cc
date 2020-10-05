// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/system_utils.h"

#include <Windows.h>

#include <sstream>

#include "flutter/shell/platform/windows/string_conversion.h"

namespace flutter {

std::vector<LanguageInfo> GetPreferredLanguageInfo() {
  std::vector<std::wstring> languages = GetPreferredLanguages();
  std::vector<LanguageInfo> language_info;
  // TODO populate via WinRT
  return language_info;
}

std::vector<std::wstring> GetPreferredLanguages() {
  std::vector<std::wstring> languages;
  // TODO populate via WinRT
  return languages;
}

LanguageInfo ParseLanguageName(std::wstring language_name) {
  LanguageInfo info;

  // TODO populate via WinRT
  return info;
}

}  // namespace flutter
