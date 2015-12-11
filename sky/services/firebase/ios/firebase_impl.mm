// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/services/firebase/ios/firebase_impl.h"
#include "base/logging.h"
#include "base/strings/sys_string_conversions.h"

#import <Firebase.h>

namespace sky {
namespace services {
namespace firebase {

FirebaseImpl::FirebaseImpl(mojo::InterfaceRequest<::firebase::Firebase> request)
    : binding_(this, request.Pass()) {}

FirebaseImpl::~FirebaseImpl() {
  [client_ release];
}

void FirebaseImpl::InitWithUrl(const mojo::String& url) {
  client_ = [[[::Firebase alloc] initWithUrl:@(url.data())] retain];
}

void FirebaseImpl::GetRoot(mojo::InterfaceRequest<::firebase::Firebase> request) {
  FirebaseImpl *root = new FirebaseImpl(request.Pass());
  root->client_ = [[client_ root] retain];
}

void FirebaseImpl::GetChild(
    const mojo::String& path,
    mojo::InterfaceRequest<Firebase> request) {
  FirebaseImpl *child = new FirebaseImpl(request.Pass());
  child->client_ = [[client_ childByAppendingPath:@(path.data())] retain];
}

void FirebaseImpl::ObserveSingleEventOfType(
    ::firebase::EventType eventType,
    const ObserveSingleEventOfTypeCallback& callback) {
  ObserveSingleEventOfTypeCallback *copyCallback =
    new ObserveSingleEventOfTypeCallback(callback);
  callback.Run(nullptr);
  [client_ observeSingleEventOfType:static_cast<FEventType>(eventType)
                          withBlock:^(FDataSnapshot *snapshot) {
    ::firebase::DataSnapshotPtr mojoSnapshot(::firebase::DataSnapshot::New());
    mojoSnapshot->key = base::SysNSStringToUTF8([snapshot key]);
    copyCallback->Run(mojoSnapshot.Pass());
    delete copyCallback;
  }];
}

void FirebaseFactory::Create(
    mojo::ApplicationConnection* connection,
    mojo::InterfaceRequest<::firebase::Firebase> request) {
  new FirebaseImpl(request.Pass());
}

}  // namespace firebase
}  // namespace services
}  // namespace sky
