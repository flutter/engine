// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

part of flutter_gpu;

base class CommandBuffer extends NativeFieldWrapperClass1 {
  /// Creates a new CommandBuffer.
  CommandBuffer._(GpuContext gpuContext) {
    _initialize(gpuContext);
  }

  RenderPass createRenderPass(
      {required ColorAttachment colorAttachment,
      StencilAttachment? stencilAttachment = null}) {
    return RenderPass._(this, colorAttachment, stencilAttachment);
  }

  /// Wrap with native counterpart.
  @Native<Bool Function(Handle, Pointer<Void>)>(
      symbol: 'InternalFlutterGpu_CommandBuffer_Initialize')
  external bool _initialize(GpuContext gpuContext);
}
