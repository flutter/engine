// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_GLFW_FLUTTER_GLFW_PRIVATE_H_
#define FLUTTER_SHELL_PLATFORM_GLFW_FLUTTER_GLFW_PRIVATE_H_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "flutter/shell/platform/embedder/embedder.h"

const char* GetLocaleStringFromEnvironmentVariables(const char* language,
                                                    const char* lc_all,
                                                    const char* lc_messages,
                                                    const char* lang);

void ParseLocale(const std::string& locale,
                 std::string* language,
                 std::string* territory,
                 std::string* codeset,
                 std::string* modifier);

std::vector<std::unique_ptr<FlutterLocale>> GetLocales(
    const char* locale_string,
    std::list<std::string>& locale_storage);

#endif  // FLUTTER_SHELL_PLATFORM_GLFW_FLUTTER_GLFW_PRIVATE_H_
