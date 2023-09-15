// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/affinity.h"

#include <ctype.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include "flutter/fml/logging.h"

namespace fml {

std::once_flag flag1;
static char* data_ = nullptr;
static intptr_t datalen_ = 0;

void InitCPUInfo() {
  // Get the size of the cpuinfo file by reading it until the end. This is
  // required because files under /proc do not always return a valid size
  // when using fseek(0, SEEK_END) + ftell(). Nor can they be mmap()-ed.
  static const char PATHNAME[] = "/proc/cpuinfo";
  FILE* fp = fopen(PATHNAME, "r");
  if (fp != nullptr) {
    for (;;) {
      char buffer[256];
      size_t n = fread(buffer, 1, sizeof(buffer), fp);
      if (n == 0) {
        break;
      }
      datalen_ += n;
    }
    fclose(fp);
  }

  // Read the contents of the cpuinfo file.
  data_ = reinterpret_cast<char*>(malloc(datalen_ + 1));
  fp = fopen(PATHNAME, "r");
  if (fp != nullptr) {
    for (intptr_t offset = 0; offset < datalen_;) {
      size_t n = fread(data_ + offset, 1, datalen_ - offset, fp);
      if (n == 0) {
        break;
      }
      offset += n;
    }
    fclose(fp);
  }

  // Zero-terminate the data.
  data_[datalen_] = '\0';
}

char* FieldStart(const char* field) {
  // Look for first field occurrence, and ensure it starts the line.
  size_t fieldlen = strlen(field);
  char* p = data_;
  for (;;) {
    p = strstr(p, field);
    if (p == nullptr) {
      return nullptr;
    }
    if (p == data_ || p[-1] == '\n') {
      break;
    }
    p += fieldlen;
  }

  // Skip to the first colon followed by a space.
  p = strchr(p + fieldlen, ':');
  if (p == nullptr || (isspace(p[1]) == 0)) {
    return nullptr;
  }
  p += 2;

  return p;
}

bool FieldContains(const char* field, const char* search_string) {
  FML_DCHECK(data_ != nullptr);
  FML_DCHECK(search_string != nullptr);

  char* p = FieldStart(field);
  if (p == nullptr) {
    return false;
  }

  // Find the end of the line.
  char* q = strchr(p, '\n');
  if (q == nullptr) {
    q = data_ + datalen_;
  }

  char saved_end = *q;
  *q = '\0';
  bool ret = (strcasestr(p, search_string) != nullptr);
  *q = saved_end;

  return ret;
}

// Extract the content of a the first occurrence of a given field in
// the content of the cpuinfo file and return it as a heap-allocated
// string that must be freed by the caller using free.
// Return nullptr if not found.
const char* ExtractField(const char* field) {
  FML_DCHECK(field != nullptr);
  FML_DCHECK(data_ != nullptr);

  char* p = FieldStart(field);
  if (p == nullptr) {
    return nullptr;
  }

  // Find the end of the line.
  char* q = strchr(p, '\n');
  if (q == nullptr) {
    q = data_ + datalen_;
  }

  intptr_t len = q - p;
  char* result = reinterpret_cast<char*>(malloc(len + 1));
  // Copy the line into result, leaving enough room for a null-terminator.
  char saved_end = *q;
  *q = '\0';
  strncpy(result, p, len);
  result[len] = '\0';
  *q = saved_end;

  return result;
}

bool HasField(const char* field) {
  FML_DCHECK(field != nullptr);
  FML_DCHECK(data_ != nullptr);
  return (FieldStart(field) != nullptr);
}

bool RequestAffinity(CpuAffinity affinity) {
  // Populate CPU Info if undefined.
  std::call_once(flag1, InitCPUInfo);

  // Determine physical core count and speed.
  std::vector<int> slow_cores;
  std::vector<int> fast_cores;

  // If we either cannot determine slow versus fast cores, or if there is no
  // distinction in core speed, return without setting affinity.
  if (slow_cores.size() == 0u || fast_cores.size() == 0u) {
    FML_LOG(ERROR) << "early reurn";
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

}  // namespace fml
