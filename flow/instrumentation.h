// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_INSTRUMENTATION_H_
#define FLUTTER_FLOW_INSTRUMENTATION_H_

#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/time/time_point.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace flutter {

class Stopwatch {
 public:
  /// The update type that is passed to the Stopwatch's constructor.
  enum FrameBudgetUpdateType {
    /// Update the latest frame budget value from refresh rate everytime.
    /// See: `DisplayManager::GetMainDisplayRefreshRate`
    kUpdateEverytime,
    /// Using the default value `fml::kDefaultFrameBudget` (60 fps)
    kOnShotValue,
  };

  /// The delegate interface for `Stopwatch`.
  class Delegate {
   public:
    /// Time limit for a smooth frame.
    /// See: `DisplayManager::GetMainDisplayRefreshRate`.
    virtual fml::Milliseconds GetFrameBudget() = 0;
  };

  /// The constructor to save the on-shot initial_frame_budget value.
  explicit Stopwatch(
      fml::Milliseconds initial_frame_budget = fml::kDefaultFrameBudget);

  /// The constructor with a delegate parameter, it will update frame_budget
  /// everytime when `GetFrameBudget()` is called.
  explicit Stopwatch(Delegate* delegate);

  ~Stopwatch();

  const fml::TimeDelta& LastLap() const;

  fml::TimeDelta MaxDelta() const;

  fml::TimeDelta AverageDelta() const;

  void InitVisualizeSurface(const SkRect& rect) const;

  void Visualize(SkCanvas* canvas, const SkRect& rect) const;

  void Start();

  void Stop();

  void SetLapTime(const fml::TimeDelta& delta);

  /// All places which want to get frame_budget should call this function.
  fml::Milliseconds GetFrameBudget() const;

 private:
  /// The private constructor which is called by the public constructors, it
  /// includes the common initializing logic.
  explicit Stopwatch(FrameBudgetUpdateType frame_budget_update_type,
                     Delegate* delegate,
                     fml::Milliseconds initial_frame_budget);

  inline double UnitFrameInterval(double time_ms) const;
  inline double UnitHeight(double time_ms, double max_height) const;

  FrameBudgetUpdateType frame_budget_update_type_;
  Delegate* delegate_;
  fml::TimePoint start_;
  std::vector<fml::TimeDelta> laps_;
  size_t current_sample_;
  fml::Milliseconds initial_frame_budget_;

  // Mutable data cache for performance optimization of the graphs. Prevents
  // expensive redrawing of old data.
  mutable bool cache_dirty_;
  mutable sk_sp<SkSurface> visualize_cache_surface_;
  mutable size_t prev_drawn_sample_index_;

  FML_DISALLOW_COPY_AND_ASSIGN(Stopwatch);
};

class Counter {
 public:
  Counter() : count_(0) {}

  size_t count() const { return count_; }

  void Reset(size_t count = 0) { count_ = count; }

  void Increment(size_t count = 1) { count_ += count; }

 private:
  size_t count_;

  FML_DISALLOW_COPY_AND_ASSIGN(Counter);
};

class CounterValues {
 public:
  CounterValues();

  ~CounterValues();

  void Add(int64_t value);

  void Visualize(SkCanvas* canvas, const SkRect& rect) const;

  int64_t GetMaxValue() const;

  int64_t GetMinValue() const;

 private:
  std::vector<int64_t> values_;
  size_t current_sample_;

  FML_DISALLOW_COPY_AND_ASSIGN(CounterValues);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_INSTRUMENTATION_H_
