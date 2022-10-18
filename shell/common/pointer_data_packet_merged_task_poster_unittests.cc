// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pointer_data_packet_merged_task_poster.h"

#include "gtest/gtest.h"
#include "thread_host.h"

namespace flutter {
namespace testing {

static void CreateSimplePointerData(PointerData& data, double dx) {
  data.time_stamp = 0;
  data.change = PointerData::Change::kMove;
  data.kind = PointerData::DeviceKind::kTouch;
  data.signal_kind = PointerData::SignalKind::kNone;
  data.device = 0;
  data.pointer_identifier = 0;
  data.physical_x = dx;
  data.physical_y = 0;
  data.physical_delta_x = 0.0;
  data.physical_delta_y = 0.0;
  data.buttons = 0;
  data.obscured = 0;
  data.synthesized = 0;
  data.pressure = 0.0;
  data.pressure_min = 0.0;
  data.pressure_max = 0.0;
  data.distance = 0.0;
  data.distance_max = 0.0;
  data.size = 0.0;
  data.radius_major = 0.0;
  data.radius_minor = 0.0;
  data.radius_min = 0.0;
  data.radius_max = 0.0;
  data.orientation = 0.0;
  data.tilt = 0.0;
  data.platformData = 0;
  data.scroll_delta_x = 0.0;
  data.scroll_delta_y = 0.0;
}

static std::unique_ptr<PointerDataPacket> CreateSimplePointerDataPacket(
    std::vector<double> dx_values) {
  std::unique_ptr<PointerDataPacket> ans =
      std::make_unique<PointerDataPacket>(dx_values.size());
  for (size_t i = 0; i < dx_values.size(); ++i) {
    PointerData data;
    CreateSimplePointerData(data, dx_values[i]);
    ans->SetPointerData(i, data);
  }
  return ans;
}

// Stolen from pointer_data_packet_converter_unittests.cc.
static void UnpackPointerPacket(
    std::vector<flutter::PointerData>& output,
    std::unique_ptr<flutter::PointerDataPacket> packet) {
  size_t kBytesPerPointerData =
      flutter::kPointerDataFieldCount * flutter::kBytesPerField;
  auto buffer = packet->data();
  size_t buffer_length = buffer.size();

  for (size_t i = 0; i < buffer_length / kBytesPerPointerData; i++) {
    flutter::PointerData pointer_data;
    memcpy(&pointer_data, &buffer[i * kBytesPerPointerData],
           sizeof(flutter::PointerData));
    output.push_back(pointer_data);
  }
  packet.reset();
}

static std::vector<double> ExtractDxValuesFromPointerDataPacket(
    std::unique_ptr<flutter::PointerDataPacket> packet) {
  std::vector<flutter::PointerData> events;
  UnpackPointerPacket(events, std::move(packet));

  std::vector<double> dx_values;
  for (auto& event : events) {
    dx_values.push_back(event.physical_x);
  }
  return dx_values;
}

TEST(PointerDataPacketMergedTaskPosterTest, DispatchOneByOne) {
  std::unique_ptr<ThreadHost> thread_host =
      std::make_unique<ThreadHost>(ThreadHost::ThreadHostConfig(
          "io.flutter.bench.",
          ThreadHost::Type::Platform | ThreadHost::Type::RASTER |
              ThreadHost::Type::IO | ThreadHost::Type::UI));
  PointerDataPacketMergedTaskPoster task_poster;
  fml::AutoResetWaitableEvent callback_latch;

  task_poster.Dispatch(
      CreateSimplePointerDataPacket({1.0, 2.0, 3.0}),
      thread_host->ui_thread->GetTaskRunner(),
      [&](std::unique_ptr<PointerDataPacket> packet) {
        FML_LOG(ERROR) << "callback 1 called";
        std::vector<double> actual_dx_values =
            ExtractDxValuesFromPointerDataPacket(std::move(packet));
        std::vector<double> expect_dx_values = {1.0, 2.0, 3.0};
        EXPECT_EQ(actual_dx_values, expect_dx_values);
        callback_latch.Signal();
      });

  FML_LOG(ERROR) << "main thread wait latch 1";
  callback_latch.Wait();

  task_poster.Dispatch(
      CreateSimplePointerDataPacket({4.0, 5.0}),
      thread_host->ui_thread->GetTaskRunner(),
      [&](std::unique_ptr<PointerDataPacket> packet) {
        FML_LOG(ERROR) << "callback 2 called";
        std::vector<double> actual_dx_values =
            ExtractDxValuesFromPointerDataPacket(std::move(packet));
        std::vector<double> expect_dx_values = {4.0, 5.0};
        EXPECT_EQ(actual_dx_values, expect_dx_values);
        callback_latch.Signal();
      });

  FML_LOG(ERROR) << "main thread wait latch 2";
  callback_latch.Wait();
}

TEST(PointerDataPacketMergedTaskPosterTest, DispatchTwice) {
  std::unique_ptr<ThreadHost> thread_host =
      std::make_unique<ThreadHost>(ThreadHost::ThreadHostConfig(
          "io.flutter.bench.",
          ThreadHost::Type::Platform | ThreadHost::Type::RASTER |
              ThreadHost::Type::IO | ThreadHost::Type::UI));
  PointerDataPacketMergedTaskPoster task_poster;
  fml::AutoResetWaitableEvent callback_latch;

  fml::AutoResetWaitableEvent blocker_task_latch;
  thread_host->ui_thread->GetTaskRunner()->PostTask([&] {
    FML_LOG(ERROR) << "blocker_task_latch wait start";
    blocker_task_latch.Wait();
    FML_LOG(ERROR) << "blocker_task_latch wait end";
  });

  task_poster.Dispatch(
      CreateSimplePointerDataPacket({1.0, 2.0, 3.0}),
      thread_host->ui_thread->GetTaskRunner(),
      [&](std::unique_ptr<PointerDataPacket> packet) {
        FML_LOG(ERROR) << "callback called";
        std::vector<double> actual_dx_values =
            ExtractDxValuesFromPointerDataPacket(std::move(packet));
        std::vector<double> expect_dx_values = {1.0, 2.0, 3.0, 4.0, 5.0};
        EXPECT_EQ(actual_dx_values, expect_dx_values);
        callback_latch.Signal();
      });

  task_poster.Dispatch(
      CreateSimplePointerDataPacket({4.0, 5.0}),
      thread_host->ui_thread->GetTaskRunner(),
      [&](std::unique_ptr<PointerDataPacket> packet) { FAIL(); });

  FML_LOG(ERROR) << "main thread signal blocker_task_latch";
  blocker_task_latch.Signal();

  FML_LOG(ERROR) << "main thread wait latch";
  callback_latch.Wait();
}

}  // namespace testing
}  // namespace flutter
