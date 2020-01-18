// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "flutter/shell/common/pointer_data_dispatcher.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

class TestPointerDataDispatcherDelegate
    : public PointerDataDispatcher::Delegate {
 public:
  std::vector<PointerData>& pointer_data() { return pointer_data_; }
  bool has_callback() { return !!callback_; }

  void FireCallback(int64_t frame_time_us) {
    auto callback = std::move(callback_);
    auto frame_time = fml::TimePoint::FromEpochDelta(
        fml::TimeDelta::FromMicroseconds(frame_time_us));
    callback(frame_time, frame_time);
  }

  // |PointerDataDispatcer::Delegate|
  void DoDispatchPacket(std::unique_ptr<PointerDataPacket> packet,
                        uint64_t trace_flow_id) override {
    size_t pointer_data_count = packet->data().size() / sizeof(PointerData);
    for (size_t i = 0; i < pointer_data_count; i++) {
      PointerData data;
      packet->GetPointerData(i, &data);
      pointer_data_.push_back(data);
    }
  }

  // |PointerDataDispatcer::Delegate|
  void ScheduleSecondaryVsyncCallback(
      const VsyncWaiter::Callback& callback) override {
    callback_ = callback;
  }

 private:
  std::vector<PointerData> pointer_data_;
  VsyncWaiter::Callback callback_;
};

void CreateSimulatedPointerData(PointerData& data,
                                PointerData::Change change,
                                int64_t time_stamp,
                                double x,
                                double y,
                                double delta_x,
                                double delta_y) {
  data.Clear();
  data.time_stamp = time_stamp;
  data.change = change;
  data.kind = PointerData::DeviceKind::kTouch;
  data.signal_kind = PointerData::SignalKind::kNone;
  data.physical_x = x;
  data.physical_y = y;
  data.physical_delta_x = delta_x;
  data.physical_delta_y = delta_y;
}

TEST(PointerDataDispatcherTest, Resampling) {
  const int64_t kSampleOffset = -500;

  TestPointerDataDispatcherDelegate delegate;
  ResamplingPointerDataDispatcher dispatcher(delegate, kSampleOffset);

  auto packet = std::make_unique<PointerDataPacket>(4);
  PointerData data;
  CreateSimulatedPointerData(data, PointerData::Change::kDown, 1000, 0.0, 0.0,
                             0.0, 0.0);
  packet->SetPointerData(0, data);
  CreateSimulatedPointerData(data, PointerData::Change::kMove, 2000, 10.0, 0.0,
                             10.0, 0.0);
  packet->SetPointerData(1, data);
  CreateSimulatedPointerData(data, PointerData::Change::kMove, 3000, 20.0, 0.0,
                             10.0, 0.0);
  packet->SetPointerData(2, data);
  CreateSimulatedPointerData(data, PointerData::Change::kUp, 4000, 30.0, 0.0,
                             10.0, 0.0);
  packet->SetPointerData(3, data);
  dispatcher.DispatchPacket(std::move(packet), 0);

  // No pointer data should have been dispatched yet.
  EXPECT_EQ(delegate.pointer_data().size(), 0u);
  // Callback should be set.
  ASSERT_EQ(delegate.has_callback(), true);

  int64_t frame_time = 2000;
  delegate.FireCallback(frame_time);

  // Two pointer changes should have been dispatched.
  auto result = delegate.pointer_data();
  ASSERT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0].time_stamp, 1000);
  EXPECT_EQ(result[0].change, PointerData::Change::kAdd);
  EXPECT_EQ(result[0].physical_x, 0.0);
  EXPECT_EQ(result[0].physical_y, 0.0);
  EXPECT_EQ(result[0].physical_delta_x, 0.0);
  EXPECT_EQ(result[0].physical_delta_y, 0.0);
  EXPECT_EQ(result[1].time_stamp, 1000);
  EXPECT_EQ(result[1].change, PointerData::Change::kDown);
  EXPECT_EQ(result[1].physical_x, 0.0);
  EXPECT_EQ(result[1].physical_y, 0.0);
  EXPECT_EQ(result[1].physical_delta_x, 0.0);
  EXPECT_EQ(result[1].physical_delta_y, 0.0);
  EXPECT_EQ(result[2].time_stamp, frame_time + kSampleOffset);
  EXPECT_EQ(result[2].change, PointerData::Change::kMove);
  EXPECT_EQ(result[2].physical_x, 5.0);
  EXPECT_EQ(result[2].physical_y, 0.0);
  EXPECT_EQ(result[2].physical_delta_x, 5.0);
  EXPECT_EQ(result[2].physical_delta_y, 0.0);

  // Callback should still be set.
  ASSERT_EQ(delegate.has_callback(), true);

  frame_time = 4000;
  delegate.FireCallback(frame_time);

  // Another pointer change should have been dispatched.
  result = delegate.pointer_data();
  ASSERT_EQ(result.size(), 4u);
  EXPECT_EQ(result[3].time_stamp, frame_time + kSampleOffset);
  EXPECT_EQ(result[3].change, PointerData::Change::kMove);
  EXPECT_EQ(result[3].physical_x, 25.0);
  EXPECT_EQ(result[3].physical_y, 0.0);
  EXPECT_EQ(result[3].physical_delta_x, 20.0);
  EXPECT_EQ(result[3].physical_delta_y, 0.0);

  frame_time = 6000;
  delegate.FireCallback(frame_time);

  // Last pointer change should have been dispatched.
  result = delegate.pointer_data();
  ASSERT_EQ(result.size(), 6u);
  EXPECT_EQ(result[4].time_stamp, 4000);
  EXPECT_EQ(result[4].change, PointerData::Change::kMove);
  EXPECT_EQ(result[4].physical_x, 30.0);
  EXPECT_EQ(result[4].physical_y, 0.0);
  EXPECT_EQ(result[4].physical_delta_x, 5.0);
  EXPECT_EQ(result[4].physical_delta_y, 0.0);
  EXPECT_EQ(result[5].time_stamp, 4000);
  EXPECT_EQ(result[5].change, PointerData::Change::kUp);
  EXPECT_EQ(result[5].physical_x, 30.0);
  EXPECT_EQ(result[5].physical_y, 0.0);

  // Callback should no longer be set.
  ASSERT_EQ(delegate.has_callback(), false);
}

}  // namespace testing
}  // namespace flutter
