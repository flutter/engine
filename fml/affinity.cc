// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/affinity.h"

#include <sched.h>
#include <pthread.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <thread>
#include <vector>

#include "flutter/fml/logging.h"

namespace fml {

#if FML_OS_ANDROID

struct CpuIndexAndSpeed {
  size_t index;
  int32_t speed;
};

class CPUSpeedTracker {
 public:
  explicit CPUSpeedTracker(std::vector<CpuIndexAndSpeed> data)
      : cpu_speeds_(data) {}

 private:
  std::vector<CpuIndexAndSpeed> cpu_speeds_;
};

std::vector<CpuIndexAndSpeed> InitCPUInfo(size_t cpu_count) {
  std::vector<CpuIndexAndSpeed> cpu_speeds;
  FML_LOG(ERROR) << "CPU COUNT: " << cpu_count;

  // Get the size of the cpuinfo file by reading it until the end. This is
  // required because files under /proc do not always return a valid size
  // when using fseek(0, SEEK_END) + ftell(). Nor can they be mmap()-ed.
  for (auto i = 0u; i < cpu_count; i++) {
    int data_length = 0;
    auto path = "/sys/devices/system/cpu/cpu" + std::to_string(i) +
                "/cpufreq/cpuinfo_max_freq";
    FML_LOG(ERROR) << path;
    FILE* fp = fopen(path.c_str(), "r");
    if (fp != nullptr) {
      for (;;) {
        char buffer[256];
        size_t n = fread(buffer, 1, sizeof(buffer), fp);
        if (n == 0) {
          break;
        }
        data_length += n;
      }
      fclose(fp);
    }

    // Read the contents of the cpuinfo file.
    char* data = reinterpret_cast<char*>(malloc(data_length + 1));
    fp = fopen(path.c_str(), "r");
    if (fp != nullptr) {
      for (intptr_t offset = 0; offset < data_length;) {
        size_t n = fread(data + offset, 1, data_length - offset, fp);
        if (n == 0) {
          break;
        }
        offset += n;
      }
      fclose(fp);
    }
     FML_LOG(ERROR) << data;

    auto speed = std::stoi(data);
    if (speed > 0) {
      cpu_speeds.push_back({.index = i, .speed = speed});
    }
  }

  for (const auto data : cpu_speeds) {
    FML_LOG(ERROR) << "CPU INDEX: " << data.index << " " << data.speed;
  }

  return cpu_speeds;
}

bool RequestAffinity(CpuAffinity affinity) {
  // Populate CPU Info if undefined.
  auto data = InitCPUInfo(std::thread::hardware_concurrency());

  // Determine physical core count and speed.

  // If we either cannot determine slow versus fast cores, or if there is no
  // distinction in core speed, return without setting affinity.
  if (data.size() == 0u) {
    return true;
  }

  cpu_set_t set;
  auto count = 8;  // TODO.
  CPU_ZERO(&set);

  switch (affinity) {
    case CpuAffinity::kPerformance:
      // Assume the last N cores are performance;
      for (auto i = 0; i < count / 2; i++) {
        CPU_SET(i, &set);
      }
      break;
    case CpuAffinity::kEfficiency:
      // Assume the first N cores are efficiency;
      for (auto i = count / 2; i < count; i++) {
        CPU_SET(i, &set);
      }
      break;
  }
  return sched_setaffinity(gettid(), sizeof(set), &set) == 0;
}

#else

bool RequestAffinity(CpuAffinity affinity) {
  return false;
}

#endif

}  // namespace fml
