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
  Dart_Handle ioLibrary = Dart_LookupLibrary(ToDart("dart:io"));
  Dart_Handle result = Dart_SetNativeResolver(
      ioLibrary, dart::bin::LookupIONative, dart::bin::LookupIONativeSymbol);
  FML_CHECK(!LogIfError(result));

  Dart_Handle allow_insecure_connections_result =
      Dart_SetField(ioLibrary, ToDart("_mayInsecurelyConnectToAllDomains"),
                    ToDart(may_insecurely_connect_to_all_domains));
  FML_CHECK(!LogIfError(allow_insecure_connections_result));

  Dart_Handle dart_args[1];
  dart_args[0] = ToDart(domain_network_policy);
  Dart_Handle set_domain_network_policy_result =
      Dart_Invoke(ioLibrary, ToDart("_setDomainPolicies"), 1, dart_args);
  FML_CHECK(!LogIfError(set_domain_network_policy_result));
}

}  // namespace flutter
