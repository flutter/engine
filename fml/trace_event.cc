// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/trace_event.h"

#include <algorithm>
#include <atomic>
#include <utility>

#include "flutter/fml/build_config.h"
#include "flutter/fml/logging.h"

#ifdef FLUTTER_TRACE_STDOUT
#include <chrono>

namespace {
int64_t GetCurrentTimeMS() {
  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>
      now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  std::chrono::duration<int64_t, std::ratio<1, 1000>> epoch =
      now_ms.time_since_epoch();
  return epoch.count();
}
}  // namespace

#define FLUTTER_TRACE_LOG(PREFIX, CATEGORY_GROUP, NAME) \
  printf("[%lld] %s%s:%s\n", GetCurrentTimeMS(), PREFIX, CATEGORY_GROUP, NAME);
#else
#define FLUTTER_TRACE_LOG(PREFIX, CATEGORY_GROUP, NAME)
#endif

namespace fml {
namespace tracing {

size_t TraceNonce() {
  static std::atomic_size_t gLastItem;
  return ++gLastItem;
}

void TraceTimelineEvent(TraceArg category_group,
                        TraceArg name,
                        TraceIDArg identifier,
                        Dart_Timeline_Event_Type type,
                        const std::vector<const char*>& c_names,
                        const std::vector<std::string>& values) {
  FLUTTER_TRACE_LOG("", category_group, name);
  const auto argument_count = std::min(c_names.size(), values.size());

  std::vector<const char*> c_values;
  c_values.resize(argument_count, nullptr);

  for (size_t i = 0; i < argument_count; i++) {
    c_values[i] = values[i].c_str();
  }

  Dart_TimelineEvent(
      name,                                      // label
      Dart_TimelineGetMicros(),                  // timestamp0
      identifier,                                // timestamp1_or_async_id
      type,                                      // event type
      argument_count,                            // argument_count
      const_cast<const char**>(c_names.data()),  // argument_names
      c_values.data()                            // argument_values
  );
}

void TraceEvent0(TraceArg category_group, TraceArg name) {
  FLUTTER_TRACE_LOG("", category_group, name);
  Dart_TimelineEvent(name,                       // label
                     Dart_TimelineGetMicros(),   // timestamp0
                     0,                          // timestamp1_or_async_id
                     Dart_Timeline_Event_Begin,  // event type
                     0,                          // argument_count
                     nullptr,                    // argument_names
                     nullptr                     // argument_values
  );
}

void TraceEvent1(TraceArg category_group,
                 TraceArg name,
                 TraceArg arg1_name,
                 TraceArg arg1_val) {
  FLUTTER_TRACE_LOG("", category_group, name);
  const char* arg_names[] = {arg1_name};
  const char* arg_values[] = {arg1_val};
  Dart_TimelineEvent(name,                       // label
                     Dart_TimelineGetMicros(),   // timestamp0
                     0,                          // timestamp1_or_async_id
                     Dart_Timeline_Event_Begin,  // event type
                     1,                          // argument_count
                     arg_names,                  // argument_names
                     arg_values                  // argument_values
  );
}

void TraceEvent2(TraceArg category_group,
                 TraceArg name,
                 TraceArg arg1_name,
                 TraceArg arg1_val,
                 TraceArg arg2_name,
                 TraceArg arg2_val) {
  FLUTTER_TRACE_LOG("", category_group, name);
  const char* arg_names[] = {arg1_name, arg2_name};
  const char* arg_values[] = {arg1_val, arg2_val};
  Dart_TimelineEvent(name,                       // label
                     Dart_TimelineGetMicros(),   // timestamp0
                     0,                          // timestamp1_or_async_id
                     Dart_Timeline_Event_Begin,  // event type
                     2,                          // argument_count
                     arg_names,                  // argument_names
                     arg_values                  // argument_values
  );
}

void TraceEventEnd(TraceArg name) {
  FLUTTER_TRACE_LOG("event end - ", "", name);
  Dart_TimelineEvent(name,                      // label
                     Dart_TimelineGetMicros(),  // timestamp0
                     0,                         // timestamp1_or_async_id
                     Dart_Timeline_Event_End,   // event type
                     0,                         // argument_count
                     nullptr,                   // argument_names
                     nullptr                    // argument_values
  );
}

void TraceEventAsyncComplete(TraceArg category_group,
                             TraceArg name,
                             TimePoint begin,
                             TimePoint end) {
  FLUTTER_TRACE_LOG("async complete - ", category_group, name);
  auto identifier = TraceNonce();

  if (begin > end) {
    std::swap(begin, end);
  }

  Dart_TimelineEvent(name,                                   // label
                     begin.ToEpochDelta().ToMicroseconds(),  // timestamp0
                     identifier,                       // timestamp1_or_async_id
                     Dart_Timeline_Event_Async_Begin,  // event type
                     0,                                // argument_count
                     nullptr,                          // argument_names
                     nullptr                           // argument_values
  );
  Dart_TimelineEvent(name,                                 // label
                     end.ToEpochDelta().ToMicroseconds(),  // timestamp0
                     identifier,                     // timestamp1_or_async_id
                     Dart_Timeline_Event_Async_End,  // event type
                     0,                              // argument_count
                     nullptr,                        // argument_names
                     nullptr                         // argument_values
  );
}

void TraceEventAsyncBegin0(TraceArg category_group,
                           TraceArg name,
                           TraceIDArg id) {
  FLUTTER_TRACE_LOG("async begin - ", category_group, name);
  Dart_TimelineEvent(name,                             // label
                     Dart_TimelineGetMicros(),         // timestamp0
                     id,                               // timestamp1_or_async_id
                     Dart_Timeline_Event_Async_Begin,  // event type
                     0,                                // argument_count
                     nullptr,                          // argument_names
                     nullptr                           // argument_values
  );
}

void TraceEventAsyncEnd0(TraceArg category_group,
                         TraceArg name,
                         TraceIDArg id) {
  FLUTTER_TRACE_LOG("async end - ", category_group, name);
  Dart_TimelineEvent(name,                           // label
                     Dart_TimelineGetMicros(),       // timestamp0
                     id,                             // timestamp1_or_async_id
                     Dart_Timeline_Event_Async_End,  // event type
                     0,                              // argument_count
                     nullptr,                        // argument_names
                     nullptr                         // argument_values
  );
}

void TraceEventAsyncBegin1(TraceArg category_group,
                           TraceArg name,
                           TraceIDArg id,
                           TraceArg arg1_name,
                           TraceArg arg1_val) {
  FLUTTER_TRACE_LOG("async begin - ", category_group, name);
  const char* arg_names[] = {arg1_name};
  const char* arg_values[] = {arg1_val};
  Dart_TimelineEvent(name,                             // label
                     Dart_TimelineGetMicros(),         // timestamp0
                     id,                               // timestamp1_or_async_id
                     Dart_Timeline_Event_Async_Begin,  // event type
                     1,                                // argument_count
                     arg_names,                        // argument_names
                     arg_values                        // argument_values
  );
}

void TraceEventAsyncEnd1(TraceArg category_group,
                         TraceArg name,
                         TraceIDArg id,
                         TraceArg arg1_name,
                         TraceArg arg1_val) {
  FLUTTER_TRACE_LOG("async end - ", category_group, name);
  const char* arg_names[] = {arg1_name};
  const char* arg_values[] = {arg1_val};
  Dart_TimelineEvent(name,                           // label
                     Dart_TimelineGetMicros(),       // timestamp0
                     id,                             // timestamp1_or_async_id
                     Dart_Timeline_Event_Async_End,  // event type
                     1,                              // argument_count
                     arg_names,                      // argument_names
                     arg_values                      // argument_values
  );
}

void TraceEventInstant0(TraceArg category_group, TraceArg name) {
  FLUTTER_TRACE_LOG("", category_group, name);
  Dart_TimelineEvent(name,                         // label
                     Dart_TimelineGetMicros(),     // timestamp0
                     0,                            // timestamp1_or_async_id
                     Dart_Timeline_Event_Instant,  // event type
                     0,                            // argument_count
                     nullptr,                      // argument_names
                     nullptr                       // argument_values
  );
}

void TraceEventFlowBegin0(TraceArg category_group,
                          TraceArg name,
                          TraceIDArg id) {
  FLUTTER_TRACE_LOG("flow begin - ", category_group, name);
  Dart_TimelineEvent(name,                            // label
                     Dart_TimelineGetMicros(),        // timestamp0
                     id,                              // timestamp1_or_async_id
                     Dart_Timeline_Event_Flow_Begin,  // event type
                     0,                               // argument_count
                     nullptr,                         // argument_names
                     nullptr                          // argument_values
  );
}

void TraceEventFlowStep0(TraceArg category_group,
                         TraceArg name,
                         TraceIDArg id) {
  FLUTTER_TRACE_LOG("flow step - ", category_group, name);
  Dart_TimelineEvent(name,                           // label
                     Dart_TimelineGetMicros(),       // timestamp0
                     id,                             // timestamp1_or_async_id
                     Dart_Timeline_Event_Flow_Step,  // event type
                     0,                              // argument_count
                     nullptr,                        // argument_names
                     nullptr                         // argument_values
  );
}

void TraceEventFlowEnd0(TraceArg category_group, TraceArg name, TraceIDArg id) {
  FLUTTER_TRACE_LOG("flow end - ", category_group, name);
  Dart_TimelineEvent(name,                          // label
                     Dart_TimelineGetMicros(),      // timestamp0
                     id,                            // timestamp1_or_async_id
                     Dart_Timeline_Event_Flow_End,  // event type
                     0,                             // argument_count
                     nullptr,                       // argument_names
                     nullptr                        // argument_values
  );
}

}  // namespace tracing
}  // namespace fml
