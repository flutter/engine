// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>

#include "flutter/fml/macros.h"

namespace impeller {

class ShaderLibrary;
class SamplerLibrary;
class CommandBuffer;
class PipelineLibrary;
class Allocator;
class FrameCaptor;
class WorkQueue;

class Context : public std::enable_shared_from_this<Context> {
 public:
  virtual ~Context();

  virtual bool IsValid() const = 0;

  //----------------------------------------------------------------------------
  /// @return     A resource allocator.
  ///
  virtual std::shared_ptr<Allocator> GetResourceAllocator() const = 0;

  virtual std::shared_ptr<ShaderLibrary> GetShaderLibrary() const = 0;

  virtual std::shared_ptr<SamplerLibrary> GetSamplerLibrary() const = 0;

  virtual std::shared_ptr<PipelineLibrary> GetPipelineLibrary() const = 0;

  virtual std::shared_ptr<CommandBuffer> CreateCommandBuffer() const = 0;

  virtual std::shared_ptr<WorkQueue> GetWorkQueue() const = 0;

  //----------------------------------------------------------------------------
  /// @return A frame captor for offline frame analysis during development.
  /// Importantly, for rendering backends can't capture frame, this method will
  /// return nullptr.
  ///
  virtual std::shared_ptr<FrameCaptor> GetFrameCaptor() const;

  virtual bool HasThreadingRestrictions() const;

  virtual bool SupportsOffscreenMSAA() const = 0;

  virtual bool StartCapturingFrames() const;

  virtual bool StopCapturingFrames() const;

 protected:
  Context();

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(Context);
};

}  // namespace impeller
