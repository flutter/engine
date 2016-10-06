// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart_ui;

/// Platform services provided to the application by Flutter.
///
/// Invoke a platform service by passing a request as json.

typedef void PlatformServiceReply(String json);

void platformService(String json, PlatformServiceReply callback)
    native "_platformService";
