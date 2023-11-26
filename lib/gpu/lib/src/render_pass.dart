// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

part of flutter_gpu;

base class ColorAttachment {
  ColorAttachment(
      {this.loadAction = LoadAction.clear,
      this.storeAction = StoreAction.store,
      this.clearValue = const ui.Color(0x00000000),
      required this.texture,
      this.resolveTexture = null});

  LoadAction loadAction;
  StoreAction storeAction;
  ui.Color clearValue;
  Texture texture;
  Texture? resolveTexture;
}

base class DepthStencilAttachment {
  DepthStencilAttachment(
      {this.depthLoadAction = LoadAction.clear,
      this.depthStoreAction = StoreAction.dontCare,
      this.depthClearValue = 0,
      this.stencilLoadAction = LoadAction.clear,
      this.stencilStoreAction = StoreAction.dontCare,
      this.stencilClearValue = 0,
      required this.texture});
  LoadAction depthLoadAction;
  StoreAction depthStoreAction;
  double depthClearValue;

  LoadAction stencilLoadAction;
  StoreAction stencilStoreAction;
  int stencilClearValue;

  Texture texture;
}

base class RenderTarget {
  RenderTarget({required this.colorAttachments, this.depthStencilAttachment});
  List<ColorAttachment> colorAttachments;
  DepthStencilAttachment? depthStencilAttachment;
}

base class RenderPass extends NativeFieldWrapperClass1 {
  /// Creates a new RenderPass.
  RenderPass._(CommandBuffer commandBuffer, RenderTarget renderTarget) {
    _initialize();
    String? error;
    for (final (index, color) in renderTarget.colorAttachments.indexed) {
      error = _setColorAttachment(
          index,
          color.loadAction.index,
          color.storeAction.index,
          color.clearValue.value,
          color.texture,
          color.resolveTexture);
      if (error != null) {
        throw Exception(error);
      }
    }
    if (renderTarget.depthStencilAttachment != null) {
      final ds = renderTarget.depthStencilAttachment!;
      error = _setDepthStencilAttachment(
          ds.depthLoadAction.index,
          ds.depthStoreAction.index,
          ds.depthClearValue,
          ds.stencilLoadAction.index,
          ds.stencilStoreAction.index,
          ds.stencilClearValue,
          ds.texture);
      if (error != null) {
        throw Exception(error);
      }
    }
    error = _begin(commandBuffer);
    if (error != null) {
      throw Exception(error);
    }
  }

  void bindPipeline(RenderPipeline pipeline) {
    _bindPipeline(pipeline);
  }

  void bindVertexBuffer(BufferView bufferView, int vertexCount) {
    bufferView.buffer._bindAsVertexBuffer(
        this, bufferView.offsetInBytes, bufferView.lengthInBytes, vertexCount);
  }

  void bindUniform(UniformSlot slot, BufferView bufferView) {
    bool success = bufferView.buffer._bindAsUniform(
        this, slot, bufferView.offsetInBytes, bufferView.lengthInBytes);
    if (!success) {
      throw Exception("Failed to bind uniform slot");
    }
  }

  void draw() {
    if (!_draw()) {
      throw Exception("Failed to append draw");
    }
  }

  /// Wrap with native counterpart.
  @Native<Void Function(Handle)>(
      symbol: 'InternalFlutterGpu_RenderPass_Initialize')
  external void _initialize();

  @Native<
      Handle Function(Pointer<Void>, Int, Int, Int, Int, Pointer<Void>,
          Handle)>(symbol: 'InternalFlutterGpu_RenderPass_SetColorAttachment')
  external String? _setColorAttachment(
      int colorAttachmentIndex,
      int loadAction,
      int storeAction,
      int clearColor,
      Texture texture,
      Texture? resolveTexture);

  @Native<
          Handle Function(
              Pointer<Void>, Int, Int, Float, Int, Int, Int, Pointer<Void>)>(
      symbol: 'InternalFlutterGpu_RenderPass_SetDepthStencilAttachment')
  external String? _setDepthStencilAttachment(
      int depthLoadAction,
      int depthStoreAction,
      double depthClearValue,
      int stencilLoadAction,
      int stencilStoreAction,
      int stencilClearValue,
      Texture texture);

  @Native<Handle Function(Pointer<Void>, Pointer<Void>)>(
      symbol: 'InternalFlutterGpu_RenderPass_Begin')
  external String? _begin(CommandBuffer commandBuffer);

  @Native<Void Function(Pointer<Void>, Pointer<Void>)>(
      symbol: 'InternalFlutterGpu_RenderPass_BindPipeline')
  external void _bindPipeline(RenderPipeline pipeline);

  @Native<Void Function(Pointer<Void>, Pointer<Void>, Int, Int, Int)>(
      symbol: 'InternalFlutterGpu_RenderPass_BindVertexBufferDevice')
  external void _bindVertexBufferDevice(DeviceBuffer buffer, int offsetInBytes,
      int lengthInBytes, int vertexCount);

  @Native<Void Function(Pointer<Void>, Pointer<Void>, Int, Int, Int)>(
      symbol: 'InternalFlutterGpu_RenderPass_BindVertexBufferHost')
  external void _bindVertexBufferHost(
      HostBuffer buffer, int offsetInBytes, int lengthInBytes, int vertexCount);

  @Native<Bool Function(Pointer<Void>, Int, Int, Pointer<Void>, Int, Int)>(
      symbol: 'InternalFlutterGpu_RenderPass_BindUniformDevice')
  external bool _bindUniformDevice(int stage, int slotId, DeviceBuffer buffer,
      int offsetInBytes, int lengthInBytes);

  @Native<Bool Function(Pointer<Void>, Int, Int, Pointer<Void>, Int, Int)>(
      symbol: 'InternalFlutterGpu_RenderPass_BindUniformHost')
  external bool _bindUniformHost(int stage, int slotId, HostBuffer buffer,
      int offsetInBytes, int lengthInBytes);

  @Native<Bool Function(Pointer<Void>)>(
      symbol: 'InternalFlutterGpu_RenderPass_Draw')
  external bool _draw();
}
