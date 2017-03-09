// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERBINARYMESSAGES_H_
#define FLUTTER_FLUTTERBINARYMESSAGES_H_

#import <Foundation/Foundation.h>

typedef void (^FlutterBinaryReplyHandler)(NSData* reply);
typedef void (^FlutterBinaryMessageHandler)(NSData* message, FlutterBinaryReplyHandler replyHandler);

#endif  // FLUTTER_FLUTTERBINARYMESSAGES_H_
