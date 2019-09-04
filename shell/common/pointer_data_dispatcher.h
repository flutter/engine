// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef POINTER_DATA_DISPATCHER_H_
#define POINTER_DATA_DISPATCHER_H_

#include "flutter/runtime/runtime_controller.h"
#include "flutter/shell/common/animator.h"

namespace flutter {

class PointerDataDispatcher {
 public:
  virtual void DispatchPacket(std::unique_ptr<PointerDataPacket> packet,
                              uint64_t trace_flow_id) = 0;
  virtual void OnRender() = 0;
  virtual ~PointerDataDispatcher() {}
};

class DefaultPointerDataDispatcher : public PointerDataDispatcher {
 public:
  DefaultPointerDataDispatcher(Animator& animator,
                               RuntimeController& runtime_controller)
      : runtime_controller_(runtime_controller), animator_(animator) {}

  void DispatchPacket(std::unique_ptr<PointerDataPacket> packet,
                      uint64_t trace_flow_id) override;
  void OnRender() override {}  // Intentional no-op

  virtual ~DefaultPointerDataDispatcher() {}

 protected:
  RuntimeController& runtime_controller_;
  Animator& animator_;
};

class IosPointerDataDispatcher : public DefaultPointerDataDispatcher {
 public:
  IosPointerDataDispatcher(Animator& animator,
                           RuntimeController& runtime_controller)
      : DefaultPointerDataDispatcher(animator, runtime_controller) {}

  void DispatchPacket(std::unique_ptr<PointerDataPacket> packet,
                      uint64_t trace_flow_id) override;
  void OnRender() override;

  virtual ~IosPointerDataDispatcher() {}

 private:
  // If non-null, this will be a pending pointer data packet for the next frame
  // to consume. This is used to smooth out the irregular drag events delivery.
  // See also `DispatchPointerDataPacket` and input_events_unittests.cc.
  std::unique_ptr<PointerDataPacket> pending_packet_;
  int pending_trace_flow_id_ = -1;

  bool is_pointer_data_in_progress_ = false;

  void DispatchPendingPacket();
};

}  // namespace flutter

#endif  // POINTER_DATA_DISPATCHER_H_
