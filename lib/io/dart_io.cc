// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/io/dart_io.h"

#include "flutter/fml/logging.h"
#include "third_party/dart/runtime/include/bin/dart_io_api.h"
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/logging/dart_error.h"

using tonic::LogIfError;
using tonic::ToDart;

namespace flutter {

void DartIO::InitForIsolate(bool may_insecurely_connect_to_all_domains,
                            std::string domain_network_policy) {
  // We should be setting fields on dart:io's _EmbedderConfig but they have
  // been reverted due to https://github.com/flutter/flutter/issues/72723.
}

}  // namespace flutter
