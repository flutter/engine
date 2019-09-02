// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/shell_test.h"
#include "flutter/testing/testing.h"

namespace flutter {
namespace testing {

// The following tests would only pass with ENABLE_IRREGULAR_INPUT_DELIVERY_FIX.
#ifdef ENABLE_IRREGULAR_INPUT_DELIVERY_FIX

using Generator = std::function<int(int)>;

TEST_F(ShellTest, MissAtMostOneFrameForIrregularInputEvents) {
  constexpr int frame_time = 10;
  constexpr int base_latency = 0.5 * frame_time;
  Generator extreme = [](int i) {
    return i * frame_time + base_latency +
           (i % 2 == 0 ? 0.1 * frame_time : 0.9 * frame_time);
  };
  constexpr int n = 40;
  std::vector<int> events_consumed_at_frame;
  TestSimulatedInputEvents(n, base_latency, extreme, frame_time,
                           events_consumed_at_frame);
  int frame_drawn = events_consumed_at_frame.size();
  ASSERT_GE(frame_drawn, n - 1);
}

TEST_F(ShellTest, DelayAtMostOneEventForFasterThanVSyncInputEvents) {
  constexpr int frame_time = 10;
  constexpr int base_latency = 0.2 * frame_time;
  Generator double_sampling = [](int i) {
    return i * 0.5 * frame_time + base_latency;
  };
  constexpr int n = 40;
  std::vector<int> events_consumed_at_frame;
  TestSimulatedInputEvents(n, base_latency, double_sampling, frame_time,
                           events_consumed_at_frame);

  // Draw one extra frame due to delaying a pending packet for the next frame.
  int frame_drawn = events_consumed_at_frame.size();
  ASSERT_EQ(frame_drawn, n / 2 + 1);

  for (int i = 0; i < n / 2; i += 1) {
    ASSERT_GE(events_consumed_at_frame[i], 2 * i - 1);
  }
}

TEST_F(ShellTest, HandlesActualIphoneXsInputEvents) {
  // Actual delivery times measured on iPhone Xs, in the unit of frame_time
  // (16.67ms for 60Hz).
  constexpr int n = 47;
  constexpr double iphone_xs_times[n] = {0.15,
                                         1.0773046874999999,
                                         2.1738720703124996,
                                         3.0579052734374996,
                                         4.0890087890624995,
                                         5.0952685546875,
                                         6.1251708984375,
                                         7.1253076171875,
                                         8.125927734374999,
                                         9.37248046875,
                                         10.133950195312499,
                                         11.161201171875,
                                         12.226992187499999,
                                         13.1443798828125,
                                         14.440327148437499,
                                         15.091684570312498,
                                         16.138681640625,
                                         17.126469726562497,
                                         18.1592431640625,
                                         19.371372070312496,
                                         20.033774414062496,
                                         21.021782226562497,
                                         22.070053710937497,
                                         23.325541992187496,
                                         24.119648437499997,
                                         25.084262695312496,
                                         26.077866210937497,
                                         27.036547851562496,
                                         28.035073242187497,
                                         29.081411132812498,
                                         30.066064453124998,
                                         31.089360351562497,
                                         32.086142578125,
                                         33.4618798828125,
                                         34.14697265624999,
                                         35.0513525390625,
                                         36.136025390624994,
                                         37.1618408203125,
                                         38.144472656249995,
                                         39.201123046875,
                                         40.4339501953125,
                                         41.1552099609375,
                                         42.102128906249995,
                                         43.0426318359375,
                                         44.070131835937495,
                                         45.08862304687499,
                                         46.091469726562494};
  // Everything is converted to int to avoid floating point error in
  // TestSimulatedInputEvents.
  constexpr int frame_time = 10000;
  for (double base_latency_f = 0; base_latency_f < 1; base_latency_f += 0.1) {
    int base_latency = static_cast<int>(base_latency_f * frame_time);
    Generator iphone_xs_generator = [iphone_xs_times, base_latency](int i) {
      return base_latency + static_cast<int>(iphone_xs_times[i] * frame_time);
    };
    std::vector<int> events_consumed_at_frame;
    TestSimulatedInputEvents(n, base_latency, iphone_xs_generator, frame_time,
                             events_consumed_at_frame);
    int frame_drawn = events_consumed_at_frame.size();
    ASSERT_GE(frame_drawn, n - 1);
  }
}

#endif  // OS_MACOXS || OS_LINUX

}  // namespace testing
}  // namespace flutter
