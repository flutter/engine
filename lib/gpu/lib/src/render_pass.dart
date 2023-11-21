// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

part of flutter_gpu;

base class ColorAttachment {
  ColorAttachment(
      {this.loadAction = LoadAction.clear,
      this.storeAction = StoreAction.store,
      this.clearColor = const ui.Color(0),
      required this.texture,
      this.resolveTexture = null});

  LoadAction loadAction;
  StoreAction storeAction;
  ui.Color clearColor;
  Texture texture;
  Texture? resolveTexture;
}

base class StencilAttachment {
  StencilAttachment(
      {this.loadAction = LoadAction.clear,
      this.storeAction = StoreAction.dontCare,
      this.clearStencil = 0,
      required this.texture});

  LoadAction loadAction;
  StoreAction storeAction;
  int clearStencil;
  Texture texture;
}

/// A descriptor for RenderPass creation. Defines the output targets for raster
/// pipelines.
base class RenderTarget {}

base class RenderPass extends NativeFieldWrapperClass1 {
  /// Creates a new RenderPass.
  RenderPass._(CommandBuffer commandBuffer, ColorAttachment colorAttachment,
      StencilAttachment? stencilAttachment) {
    _initialize();
    String? error;
    error = _setColorAttachment(
        colorAttachment.loadAction.index,
        colorAttachment.storeAction.index,
        colorAttachment.clearColor.value,
        colorAttachment.texture,
        colorAttachment.resolveTexture);
    if (error != null) {
      throw Exception(error);
    }
    if (stencilAttachment != null) {
      error = _setStencilAttachment(
          stencilAttachment.loadAction.index,
          stencilAttachment.storeAction.index,
          stencilAttachment.clearStencil,
          stencilAttachment.texture);
      if (error != null) {
        throw Exception(error);
      }
    }
    error = _begin(commandBuffer);
    if (error != null) {
      throw Exception(error);
    }
  }



  /// Wrap with native counterpart.
  @Native<Void Function(Handle)>(
      symbol: 'InternalFlutterGpu_RenderPass_Initialize')
  external void _initialize();

  @Native<Handle Function(Pointer<Void>, Int, Int, Int, Pointer<Void>, Handle)>(
      symbol: 'InternalFlutterGpu_RenderPass_SetColorAttachment')
  external String? _setColorAttachment(int loadAction, int storeAction,
      int clearColor, Texture texture, Texture? resolveTexture);

  @Native<Handle Function(Pointer<Void>, Int, Int, Int, Pointer<Void>)>(
      symbol: 'InternalFlutterGpu_RenderPass_SetStencilAttachment')
  external String? _setStencilAttachment(
      int loadAction, int storeAction, int clearStencil, Texture texture);

  @Native<Handle Function(Pointer<Void>, Pointer<Void>)>(
      symbol: 'InternalFlutterGpu_RenderPass_Begin')
  external String? _begin(CommandBuffer commandBuffer);
}
