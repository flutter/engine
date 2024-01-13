// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_PLATFORM_MESSAGE_LISTENER_DART_PORT_H_
#define FLUTTER_LIB_UI_WINDOW_PLATFORM_MESSAGE_LISTENER_DART_PORT_H_

#include "flutter/fml/mapping.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/message_loop.h"
#include "flutter/lib/ui/window/platform_message_response.h"
#include "third_party/tonic/dart_persistent_value.h"

namespace flutter {

class PlatformListenerDartPort
    : public fml::RefCountedThreadSafe<PlatformListenerDartPort> {
  FML_FRIEND_MAKE_REF_COUNTED(PlatformListenerDartPort);

 public:
  void SendEmpty();
  void Send(int response,
            const std::string& channel,
            std::unique_ptr<fml::MallocMapping> message);

 protected:
  explicit PlatformListenerDartPort(Dart_Port send_port, int64_t identifier);

  Dart_Port send_port_;
  int64_t identifier_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_PLATFORM_MESSAGE_LISTENER_DART_PORT_H_
