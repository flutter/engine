// Copyright 2016 the Flutter project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/services/gcm/src/gcm_impl.h"

#include "base/logging.h"
#include "mojo/public/c/system/types.h"
#include "sky/services/dynamic/dynamic_service_dylib.h"

namespace gcm {

GcmListenerImpl::GcmListenerImpl(mojo::InterfaceRequest<GcmListener> request)
    : binding_(this, request.Pass()) {}

GcmListenerImpl::~GcmListenerImpl() {}

void GcmListenerImpl::OnMessageReceived(const mojo::String& from,
                                        const mojo::String& json_message) {
  LOG(INFO) << "NOTIMPLEMENTED";
}

GcmServiceImpl::GcmServiceImpl(mojo::InterfaceRequest<GcmService> request)
    : binding_(this, request.Pass()) {}

GcmServiceImpl::~GcmServiceImpl() {}

void GcmServiceImpl::Register(const mojo::String& name,
                              GcmListenerPtr listener,
                              const RegisterCallback& callback) {
  LOG(INFO) << "NOTIMPLEMENTED";
  callback.Run("NOTIMPLEMENTED");
}

void GcmServiceImpl::SubscribeTopics(const mojo::String& token,
                                     mojo::Array<mojo::String> topics) {
  LOG(INFO) << "NOTIMPLEMENTED";
}

void GcmServiceImpl::UnsubscribeTopics(const mojo::String& token,
                                       mojo::Array<mojo::String> topics) {
  LOG(INFO) << "NOTIMPLEMENTED";
}

}  // namespace gcm

void FlutterServicePerform(mojo::ScopedMessagePipeHandle client_handle,
                           const mojo::String& service_name) {
  if (service_name == gcm::GcmService::Name_) {
    new gcm::GcmServiceImpl(
        mojo::MakeRequest<gcm::GcmService>(client_handle.Pass()));
    return;
  }
  if (service_name == gcm::GcmListener::Name_) {
    new gcm::GcmListenerImpl(
        mojo::MakeRequest<gcm::GcmListener>(client_handle.Pass()));
    return;
  }
}
