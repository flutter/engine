// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_SERIALIZABLE_H_
#define FLUTTER_FLOW_SERIALIZABLE_H_

#include "flutter/fml/message.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkPath.h"

namespace flow {

bool Serialize(fml::Message& message, const sk_sp<SkImageFilter>& value);
bool Deserialize(fml::Message& message, sk_sp<SkImageFilter>& value);

bool Serialize(fml::Message& message, const SkPath& value);
bool Deserialize(fml::Message& message, SkPath& value);

bool Serialize(fml::Message& message, const sk_sp<SkShader>& value);
bool Deserialize(fml::Message& message, sk_sp<SkShader>& value);

bool Serialize(fml::Message& message, const sk_sp<SkPicture>& value);
bool Deserialize(fml::Message& message, sk_sp<SkPicture>& value);

}  // namespace flow

#endif  // FLUTTER_FLOW_SERIALIZABLE_H_
