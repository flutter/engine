// Copyright 2016 the Flutter project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GCM_SRC_GCM_IMPL_H_
#define GCM_SRC_GCM_IMPL_H_

#include "mojo/public/cpp/system/macros.h"
#include "mojo/public/cpp/bindings/strong_binding.h"

#include "sky/services/gcm/gcm.mojom.h"

namespace gcm {

class GcmListenerImpl : public GcmListener {
 public:
  explicit GcmListenerImpl(mojo::InterfaceRequest<GcmListener> request);

  ~GcmListenerImpl() override;

  void OnMessageReceived(const mojo::String& from,
                         const mojo::String& json_message) override;

 private:
  mojo::StrongBinding<GcmListener> binding_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(GcmListenerImpl);
};

class GcmServiceImpl : public GcmService {
 public:
  explicit GcmServiceImpl(mojo::InterfaceRequest<GcmService> request);

  ~GcmServiceImpl() override;

  void Register(const mojo::String& sender_id,
                GcmListenerPtr listener,
                const RegisterCallback& callback) override;
  void SubscribeTopics(const mojo::String& token,
                       mojo::Array<mojo::String> topics) override;
  void UnsubscribeTopics(const mojo::String& token,
                         mojo::Array<mojo::String> topics) override;

 private:
  mojo::StrongBinding<GcmService> binding_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(GcmServiceImpl);
};

}  // namespace hello

#endif  // GCM_SRC_GCM_IMPL_H_
