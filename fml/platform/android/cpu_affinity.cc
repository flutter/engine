// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/android/cpu_affinity.h"

#include "flutter/fml/logging.h"

#include <pthread.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <mutex>
#include <optional>
#include <thread>

namespace fml {

/// The CPUSpeedTracker is initialized once the first time a thread affinity is
/// requested.
std::once_flag cpu_tracker_flag_;
static CPUSpeedTracker* tracker_;

CPUSpeedTracker::CPUSpeedTracker(std::vector<CpuIndexAndSpeed> data)
    : cpu_speeds_(std::move(data)) {
  std::optional<int32_t> max_speed = std::nullopt;
  std::optional<int32_t> min_speed = std::nullopt;
  for (const auto& data : cpu_speeds_) {
    if (!max_speed.has_value() || data.speed > max_speed.value()) {
      max_speed = data.speed;
    }
    if (!min_speed.has_value() || data.speed < min_speed.value()) {
      min_speed = data.speed;
    }
  }
  if (!max_speed.has_value() || !min_speed.has_value() ||
      min_speed.value() == max_speed.value()) {
    return;
  }

  for (const auto& data : cpu_speeds_) {
    if (data.speed == max_speed.value()) {
      performance_.push_back(data.index);
    } else {
      not_performance_.push_back(data.index);
    }
    if (data.speed == min_speed.value()) {
      efficiency_.push_back(data.index);
    }
  }

  valid_ = true;
}

bool CPUSpeedTracker::IsValid() const {
  return valid_;
}

const std::vector<size_t>& CPUSpeedTracker::GetIndices(
    CpuAffinity affinity) const {
  switch (affinity) {
    case CpuAffinity::kPerformance:
      return performance_;
    case CpuAffinity::kEfficiency:
      return efficiency_;
    case CpuAffinity::kNotPerformance:
      return not_performance_;
  }
}

// Get the size of the cpuinfo file by reading it until the end. This is
// required because files under /proc do not always return a valid size
// when using fseek(0, SEEK_END) + ftell(). Nor can they be mmap()-ed.
std::optional<int32_t> ReadIntFromFile(const std::string& path) {
  int data_length = 0u;
  FILE* fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    return std::nullopt;
  }
  for (;;) {
    char buffer[256];
    size_t n = fread(buffer, 1, sizeof(buffer), fp);
    if (n == 0) {
      break;
    }
    data_length += n;
  }
  fclose(fp);

  // Read the contents of the cpuinfo file.
  if (data_length <= 0) {
    return std::nullopt;
  }

  char* data = reinterpret_cast<char*>(malloc(data_length + 1));
  fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    return std::nullopt;
  }

  for (intptr_t offset = 0; offset < data_length;) {
    size_t n = fread(data + offset, 1, data_length - offset, fp);
    if (n == 0) {
      break;
    }
    offset += n;
  }
  fclose(fp);

  if (data == nullptr) {
    return std::nullopt;
  }

  auto speed = std::stoi(data);
  free(data);
  if (speed > 0) {
    return speed;
  }
  return std::nullopt;
}

// For each CPU index provided, attempts to open the file
// /sys/devices/system/cpu/cpu$NUM/cpufreq/cpuinfo_max_freq and parse a number
// containing the CPU frequency.
void InitCPUInfo(size_t cpu_count) {
  std::vector<CpuIndexAndSpeed> cpu_speeds;

  for (auto i = 0u; i < cpu_count; i++) {
    auto path = "/sys/devices/system/cpu/cpu" + std::to_string(i) +
                "/cpufreq/cpuinfo_max_freq";
    auto speed = ReadIntFromFile(path);
    if (speed.has_value()) {
      cpu_speeds.push_back({.index = i, .speed = speed.value()});
    }
  }
  tracker_ = new CPUSpeedTracker(cpu_speeds);
}

bool RequestAffinity(CpuAffinity affinity) {
  // Populate CPU Info if uninitialized.
  auto count = std::thread::hardware_concurrency();
  std::call_once(cpu_tracker_flag_, [count]() { InitCPUInfo(count); });
  if (tracker_ == nullptr) {
    return true;
  }

  if (!tracker_->IsValid()) {
    return true;
  }

  cpu_set_t set;
  CPU_ZERO(&set);
  for (const auto index : tracker_->GetIndices(affinity)) {
    CPU_SET(index, &set);
  }
  return sched_setaffinity(gettid(), sizeof(set), &set) == 0;
}

}  // namespace fml
