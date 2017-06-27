// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/content_handler/session_connection.h"
#include "apps/mozart/lib/scene/session_helpers.h"

namespace flutter_runner {

SessionConnection::SessionConnection(
    fidl::InterfaceHandle<mozart2::Session> session_handle,
    mx::eventpair import_token)
    : session_(mozart2::SessionPtr::Create(std::move(session_handle))),
      root_node_(&session_) {
  ASSERT_IS_GPU_THREAD;
  root_node_.Bind(std::move(import_token));
  session_.set_connection_error_handler(
      std::bind(&SessionConnection::OnSessionError, this));
  present_callback_ =
      std::bind(&SessionConnection::OnPresent, this, std::placeholders::_1);
}

SessionConnection::~SessionConnection() {
  ASSERT_IS_GPU_THREAD;
}

void SessionConnection::OnSessionError() {
  ASSERT_IS_GPU_THREAD;
  // TODO: Not this.
  FTL_CHECK(false) << "Session connection was terminated.";
}

void SessionConnection::Present(ftl::Closure on_present_callback) {
  ASSERT_IS_GPU_THREAD;
  FTL_DCHECK(pending_on_present_callback_ == nullptr);
  FTL_DCHECK(on_present_callback != nullptr);
  pending_on_present_callback_ = on_present_callback;
  session_.Present(0,                 // presentation_time. Placeholder for now.
                   present_callback_  // callback
                   );
  EnqueueClearOps();
}

void SessionConnection::OnPresent(mozart2::PresentationInfoPtr info) {
  ASSERT_IS_GPU_THREAD;
  auto callback = pending_on_present_callback_;
  pending_on_present_callback_ = nullptr;
  callback();
}

void SessionConnection::EnqueueClearOps() {
  ASSERT_IS_GPU_THREAD;
  // We are going to be sending down a fresh node hierarchy every frame. So just
  // enqueue a detach op on the imported root node.
  session_.Enqueue(mozart::NewDetachChildrenOp(root_node_.id()));
}

}  // namespace flutter_runner
