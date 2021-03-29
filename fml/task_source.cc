// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/task_source.h"

namespace fml {

TaskSource::TaskSource(TaskQueueId task_queue_id)
    : task_queue_id_(task_queue_id) {}

TaskSource::~TaskSource() = default;

void TaskSource::Clear() {
  primary_task_queue_ = {};
  secondary_task_queue_ = {};
}

void TaskSource::RegisterTask(TaskSourceGrade grade, DelayedTask&& task) {
  switch (grade) {
    case TaskSourceGrade::kPrimary:
      primary_task_queue_.push(task);
      break;
    case TaskSourceGrade::kSecondary:
      secondary_task_queue_.push(task);
      break;
  }
}

void TaskSource::PopTask(TaskSourceGrade grade) {
  switch (grade) {
    case TaskSourceGrade::kPrimary:
      primary_task_queue_.pop();
      break;
    case TaskSourceGrade::kSecondary:
      secondary_task_queue_.pop();
      break;
  }
}

size_t TaskSource::Size() const {
  size_t size = primary_task_queue_.size();
  if (secondary_pause_requests_ == 0) {
    size += secondary_task_queue_.size();
  }
  return size;
}

bool TaskSource::Empty() const {
  return Size() == 0;
}

TaskSource::TopTask TaskSource::Top() const {
  FML_CHECK(!Empty());
  if (secondary_pause_requests_ > 0 || secondary_task_queue_.empty()) {
    const auto& primary_top = primary_task_queue_.top();
    return {
        .task_queue_id = task_queue_id_,
        .task_source_grade = TaskSourceGrade::kPrimary,
        .task = primary_top,
    };
  } else if (primary_task_queue_.empty()) {
    const auto& secondary_top = secondary_task_queue_.top();
    return {
        .task_queue_id = task_queue_id_,
        .task_source_grade = TaskSourceGrade::kSecondary,
        .task = secondary_top,
    };
  } else {
    const auto& primary_top = primary_task_queue_.top();
    const auto& secondary_top = secondary_task_queue_.top();
    if (primary_top > secondary_top) {
      return {
          .task_queue_id = task_queue_id_,
          .task_source_grade = TaskSourceGrade::kSecondary,
          .task = secondary_top,
      };
    } else {
      return {
          .task_queue_id = task_queue_id_,
          .task_source_grade = TaskSourceGrade::kPrimary,
          .task = primary_top,
      };
    }
  }
}

void TaskSource::PauseSecondary() {
  secondary_pause_requests_++;
}

void TaskSource::ResumeSecondary() {
  secondary_pause_requests_--;
  FML_DCHECK(secondary_pause_requests_ >= 0);
}

}  // namespace fml
