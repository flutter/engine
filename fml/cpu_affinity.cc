// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/cpu_affinity.h"

#include <optional>
#include <sstream>
#include <string>

namespace fml {

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
  size_t data_length = 0u;
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

  if (data_length <= 0) {
    return std::nullopt;
  }

  // Read the contents of the cpuinfo file.
  char* data = reinterpret_cast<char*>(malloc(data_length + 1));
  fp = fopen(path.c_str(), "r");
  if (fp == nullptr) {
    free(data);
    return std::nullopt;
  }
  for (uintptr_t offset = 0; offset < data_length;) {
    size_t n = fread(data + offset, 1, data_length - offset, fp);
    if (n == 0) {
      break;
    }
    offset += n;
  }
  fclose(fp);

  if (data == nullptr) {
    free(data);
    return std::nullopt;
  }
  // zero end of buffer.
  data[data_length] = 0;

  // Dont use stoi because if this data isnt a parseable number then it
  // will abort, as we compile with exceptions disabled.
  int speed = 0;
  std::istringstream input(data);
  input >> speed;
  if (speed > 0) {
    return speed;
  }
  return std::nullopt;
}

}  // namespace fml
