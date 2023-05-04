// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_APPLICATION_LIFECYCLE_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_APPLICATION_LIFECYCLE_H_

namespace flutter {

/**
 * These constants describe the possible lifecycle states of the application.
 * They must be kept up to date with changes in the framework's
 * AppLifecycleState enum. They are passed to the embedder's |SetLifecycleState|
 * function.
 *
 * States not supported on a platform will be synthesized by the framework when
 * transitioning between states which are supported, so that all implementations
 * share the same state machine.
 *
 * Here is the state machine:
 *
 *     +-----------+                               +-----------+
 *     | detached  |------------------------------>|  resumed  |
 *     +-----------+                               +-----------+
 *          ^                                              ^
 *          |                                              |
 *          |                                              v
 *     +-----------+        +--------------+       +-----------+
 *     | paused    |<------>|    hidden    |<----->|  inactive |
 *     +-----------+        +--------------+       +-----------+
 */

/**
 * Corresponds to AppLifecycleState.detached: The initial state of the state
 * machine. On Android and iOS, also the final state of the state machine
 * when all views are detached. Other platforms do not enter this state again
 * after initially leaving it.
 */
constexpr const char* kAppLifecycleStateDetached = "AppLifecycleState.detached";

/**
 * Corresponds to AppLifecycleState.resumed: The nominal "running" state of
 * the
 * application. The application is visible, has input focus, and is running.
 */
constexpr const char* kAppLifecycleStateResumed = "AppLifecycleState.resumed";

/**
 * Corresponds to AppLifecycleState.inactive: At least one view of the
 * application is visible, but none have input focus. The application is
 * otherwise running normally.
 */
constexpr const char* kAppLifecycleStateInactive = "AppLifecycleState.inactive";

/**
 * Corresponds to AppLifecycleState.hidden: All views of an application are
 * hidden, either because the application is being stopped (on iOS and
 * Android), or because it is being minimized or on a desktop that is no
 * longer visible (on desktop), or on a tab that is no longer visible (on
 * web).
 */
constexpr const char* kAppLifecycleStateHidden = "AppLifecycleState.hidden";

/**
 * Corresponds to AppLifecycleState.paused: The application is not running,
 * and can be detached or started again at any time. This state is typically
 * only entered into on iOS and Android.
 */
constexpr const char* kAppLifecycleStatePaused = "AppLifecycleState.paused";

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_APPLICATION_LIFECYCLE_H_
