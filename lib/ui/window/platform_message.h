// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PLATFORM_PLATFORM_MESSAGE_H_
#define FLUTTER_LIB_UI_PLATFORM_PLATFORM_MESSAGE_H_

#include <string>
#include <vector>

#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/lib/ui/window/platform_message_response.h"

namespace flutter {

class PlatformMessage {
 public:
  PlatformMessage(std::string channel,
                  fml::NonOwnedMapping data,
                  fml::RefPtr<PlatformMessageResponse> response);
  PlatformMessage(std::string channel,
                  fml::RefPtr<PlatformMessageResponse> response);
  ~PlatformMessage();

  const std::string& channel() const { return channel_; }
  const fml::NonOwnedMapping& data() const { return data_; }
  bool hasData() { return hasData_; }

  const fml::RefPtr<PlatformMessageResponse>& response() const {
    return response_;
  }

  fml::NonOwnedMapping releaseData() { return std::move(data_); }

 private:
  std::string channel_;
  fml::NonOwnedMapping data_;
  bool hasData_;
  fml::RefPtr<PlatformMessageResponse> response_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PLATFORM_PLATFORM_MESSAGE_H_
