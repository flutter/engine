// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>

#include "impeller/renderer/command.h"
#include "impeller/renderer/compute_pipeline_descriptor.h"
#include "impeller/renderer/pipeline.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      An object used to specify compute work to the GPU along with
///             references to resources the GPU will used when doing said work.
///
///             To construct a valid command, follow these steps:
///             * Specify a valid pipeline.
///             * (Optional) Specify a debug label.
///
///             Command are very lightweight objects and can be created
///             frequently and on demand. The resources referenced in commands
///             views into buffers managed by other allocators and resource
///             managers.
///
struct ComputeCommand {
  //----------------------------------------------------------------------------
  /// The pipeline to use for this command.
  ///
  std::shared_ptr<Pipeline<ComputePipelineDescriptor>> pipeline;

  //----------------------------------------------------------------------------
  /// The offsets and size of the buffer bindings for this command.
  ///
  BindingOffsets buffer_bindings;

  //----------------------------------------------------------------------------
  /// The offsets and size of the texture bindings for this command.
  ///
  BindingOffsets texture_bindings;

#ifdef IMPELLER_DEBUG
  //----------------------------------------------------------------------------
  /// The debugging label to use for the command.
  ///
  std::string label;
#endif  // IMPELLER_DEBUG

  constexpr explicit operator bool() const {
    return pipeline && pipeline->IsValid();
  }
};

}  // namespace impeller
