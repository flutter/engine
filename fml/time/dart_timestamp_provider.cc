// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/time/dart_timestamp_provider.h"

#include "dart_tools_api.h"

namespace fml {

DartTimestampProvider::DartTimestampProvider() = default;

DartTimestampProvider::~DartTimestampProvider() = default;

fml::TimePoint DartTimestampProvider::Now() {
  return fml::TimePoint::FromTicks(Dart_TimelineGetTicks());
}

fml::TimePoint DartTimelineTicksSinceEpoch() {
  return DartTimestampProvider::Instance().Now();
}

}  // namespace fml
