// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_PARSE_URL_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_PARSE_URL_H_

#include <string>

namespace dart_runner {

// Find the last path of the component.
// fuchsia-pkg://fuchsia.com/hello_dart#meta/hello_dart.cmx -> hello_dart.cmx
std::string GetLabelFromUrl(const std::string& url);

// Find the name of the component.
// fuchsia-pkg://fuchsia.com/hello_dart#meta/hello_dart.cm -> hello_dart
std::string GetComponentNameFromUrl(const std::string& url);

}  // namespace dart_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_PARSE_URL_H_
