// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_TESTING_LOOP_FIXTURE_REAL_LOOP_H_
#define SRC_LIB_TESTING_LOOP_FIXTURE_REAL_LOOP_H_

#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/async/cpp/executor.h>
#include <lib/fit/function.h>
#include <lib/stdcompat/optional.h>
#include <lib/zx/time.h>

#include "src/lib/fxl/macros.h"

namespace loop_fixture {

class RealLoop {
 protected:
  RealLoop();
  ~RealLoop();

  // Returns the loop's asynchronous dispatch interface.
  async_dispatcher_t* dispatcher();

  // Runs the loop until it is exited.
  void RunLoop();

  // Runs the loop for at most |timeout|. Returns |true| if the timeout has been
  // reached.
  bool RunLoopWithTimeout(zx::duration timeout = zx::sec(1));

  // Runs the loop until the condition returns true.
  //
  // |step| specifies the interval at which this method should wake up to poll
  // |condition|. If |step| is |zx::duration::infinite()|, no polling timer is
  // set. Instead, the condition is checked initially and after anything happens
  // on the loop (e.g. a task executes). This is useful when the caller knows
  // that |condition| will be made true by a task running on the loop. This will
  // generally be the case unless |condition| is made true on a different
  // thread.
  void RunLoopUntil(fit::function<bool()> condition, zx::duration step = zx::msec(10));

  // Runs the loop until the condition returns true or the timeout is reached.
  // Returns |true| if the condition was met, and |false| if the timeout was
  // reached.
  //
  // |step| specifies the interval at which this method should wake up to poll
  // |condition|. If |step| is |zx::duration::infinite()|, no polling timer is
  // set. Instead, the condition is checked initially and after anything happens
  // on the loop (e.g. a task executes). This is useful when the caller knows
  // that |condition| will be made true by a task running on the loop. This will
  // generally be the case unless |condition| is made true on a different
  // thread.
  bool RunLoopWithTimeoutOrUntil(fit::function<bool()> condition, zx::duration timeout = zx::sec(1),
                                 zx::duration step = zx::msec(10));

  // Runs the message loop until idle.
  void RunLoopUntilIdle();

  // Runs the loop until the given |fit::promise| completes, and returns the
  // |fit::result| it produced.
  //
  // If the |fit::promise| never completes, this method will run forever.
  template <typename PromiseType>
  typename PromiseType::result_type RunPromise(PromiseType promise);

  // Quits the loop.
  void QuitLoop();

  // Creates a closure that quits the test message loop when executed.
  fit::closure QuitLoopClosure();

 private:
  // The message loop for the test.
  async::Loop loop_;

  FXL_DISALLOW_COPY_AND_ASSIGN(RealLoop);
};

// Internal template implementation details
// ========================================

template <typename PromiseType>
typename PromiseType::result_type RealLoop::RunPromise(PromiseType promise) {
  async::Executor e(dispatcher());
  cpp17::optional<typename PromiseType::result_type> res;
  e.schedule_task(
      promise.then([&res](typename PromiseType::result_type& v) { res = std::move(v); }));

  // We set |step| to infinity, as the automatic-wake-up timer shouldn't be
  // necessary. A well-behaved promise must always wake up the executor when
  // there's more work to be done.
  RunLoopUntil([&res]() { return res.has_value(); }, zx::duration::infinite());
  return std::move(res.value());
}

}  // namespace loop_fixture

#endif  // SRC_LIB_TESTING_LOOP_FIXTURE_REAL_LOOP_H_
