// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_SESSION_CONNECTION_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_SESSION_CONNECTION_H_

namespace flutter_runner {
using VsyncWaiterCallback = std::function<void()>;

// The component residing on the raster thread that is responsible for
// maintaining the Scenic session connection and presenting node updates.
class SessionConnection {
 public:
  virtual ~SessionConnection() = default;

  virtual void InitializeVsyncWaiterCallback(VsyncWaiterCallback callback) = 0;

  virtual bool CanRequestNewFrames() = 0;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_SESSION_CONNECTION_H_
