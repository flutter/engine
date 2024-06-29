// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/platform_message_listener_dart_port.h"

#include <array>
#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/trace_event.h"
#include "third_party/dart/runtime/include/dart_native_api.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_state.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {

PlatformListenerDartPort::PlatformListenerDartPort(Dart_Port send_port,
                                                   int64_t identifier)
    : send_port_(send_port), identifier_(identifier) {
  FML_DCHECK(send_port != ILLEGAL_PORT);
}

void PlatformListenerDartPort::SendEmpty() {
  Dart_CObject response = {
      .type = Dart_CObject_kNull,
  };
  bool did_send = Dart_PostCObject(send_port_, &response);
  FML_CHECK(did_send);
}

void PlatformListenerDartPort::Send(
    int responseid,
    const std::string& channel,
    std::unique_ptr<fml::MallocMapping> message) {
  Dart_CObject response_identifier = {
      .type = Dart_CObject_kInt64,
  };
  response_identifier.value.as_int64 = responseid;

  Dart_CObject channel_name = {
      .type = Dart_CObject_kString,
  };
  channel_name.value.as_string = channel.c_str();

  Dart_CObject response_data = {
      .type = Dart_CObject_kTypedData,
  };
  response_data.value.as_typed_data.type = Dart_TypedData_kUint8;
  response_data.value.as_typed_data.length = message->GetSize();
  response_data.value.as_typed_data.values = message->GetMapping();

  std::array<Dart_CObject*, 3> response_values = {
      &response_identifier, &channel_name, &response_data};

  Dart_CObject response = {
      .type = Dart_CObject_kArray,
  };
  response.value.as_array.length = response_values.size();
  response.value.as_array.values = response_values.data();

  bool did_send = Dart_PostCObject(send_port_, &response);
  FML_CHECK(did_send);
}

}  // namespace flutter