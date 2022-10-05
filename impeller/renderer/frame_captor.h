// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/macros.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      Frame capture configuration.
///
struct FrameCaptorConfiguration {
  /// This param is for metal backend.
  /// When this value is true, a gpu trace file will be saved in devices when
  /// capture finishes. Otherwise, the Xcode will automatically open and show
  /// trace result.
  ///
  bool mtlSaveGPUTraceDocument = false;
};

//------------------------------------------------------------------------------
/// @brief      A class used for frame capture during rendering. Backend like
///             Metal, has a ability to capture frame programmatically.
///             The frame capture should only be used in development for frame
///             offline analysis.
///
class FrameCaptor {
 public:
  virtual ~FrameCaptor();

  //----------------------------------------------------------------------------
  /// @brief      Start capturing frame. This method should only be called when
  ///             developing.
  ///
  /// @param[in]  configuration  The configuration passed in for capture.
  ///
  /// @return The operation successful or not.
  ///
  virtual bool StartCapturingFrame(FrameCaptorConfiguration configuration);

  //----------------------------------------------------------------------------
  /// @brief      Stop capturing frame. This should only be called when
  ///             developing.
  ///
  /// @return The operation successful or not.
  ///
  virtual bool StopCapturingFrame();

 protected:
  FrameCaptor();

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(FrameCaptor);
};

}  // namespace impeller
