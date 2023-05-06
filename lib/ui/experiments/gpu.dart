// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

library dart.gpu;

import 'dart:nativewrappers';

enum BlendOperation { add, subtract, reverseSubtract }

enum BlendFactor {
  zero,
  one,
  sourceColor,
  oneMinusSourceColor,
  sourceAlpha,
  oneMinusSourceAlpha,
  destinationColor,
  oneMinusDestinationColor,
  destinationAlpha,
  oneMinusDestinationAlpha,
  sourceAlphaSaturated,
  blendColor,
  oneMinusBlendColor,
  blendAlpha,
  oneMinusBlendAlpha,
}

class BlendOptions {
  const BlendOptions({
    this.colorOperation = BlendOperation.add,
    this.sourceColorFactor = BlendFactor.one,
    this.destinationColorFactor = BlendFactor.oneMinusSourceAlpha,
    this.alphaOperation = BlendOperation.add,
    this.sourceAlphaFactor = BlendFactor.one,
    this.destinationAlphaFactor = BlendFactor.oneMinusSourceAlpha,
  });

  final BlendOperation colorOperation;
  final BlendFactor sourceColorFactor;
  final BlendFactor destinationColorFactor;
  final BlendOperation alphaOperation;
  final BlendFactor sourceAlphaFactor;
  final BlendFactor destinationAlphaFactor;
}

enum StencilOperation {
  keep,
  zero,
  setToReferenceValue,
  incrementClamp,
  decrementClamp,
  invert,
  incrementWrap,
  decrementWrap,
}

enum CompareFunction {
  never,
  always,
  less,
  equal,
  lessEqual,
  greater,
  notEqual,
  greaterEqual,
}

class StencilOptions {
  const StencilOptions({
    this.operation = StencilOperation.incrementClamp,
    this.compare = CompareFunction.always,
  });

  final StencilOperation operation;
  final CompareFunction compare;
}

enum ShaderType {
  unknown,
  voidType,
  booleanType,
  signedByteType,
  unsignedByteType,
  signedShortType,
  unsignedShortType,
  signedIntType,
  unsignedIntType,
  signedInt64Type,
  unsignedInt64Type,
  atomicCounterType,
  halfFloatType,
  floatType,
  doubleType,
  structType,
  imageType,
  sampledImageType,
  samplerType,
}

class VertexAttribute {
  const VertexAttribute({
    this.name = '',
    this.location = 0,
    this.set = 0,
    this.binding = 0,
    this.type = ShaderType.floatType,
    this.elements = 2,
  });

  final String name;
  final int location;
  final int set;
  final int binding;
  final ShaderType type;
  final int elements;
}

class UniformSlot {
  const UniformSlot({
    this.name = '',
    this.set = 0,
    this.extRes0 = 0,
    this.binding = 0,
  });

  final String name;
  final int set;
  final int extRes0;
  final int binding;
}

class Shader {}

class RasterPipeline {}

/// A handle to a graphics context.
///
/// This class is created by the engine, and should not be instantiated
/// or extended directly.
///
/// To obtain the default graphics context, use [getContext].
@pragma('vm:entry-point')
class Context extends NativeFieldWrapperClass1 {
  @pragma('vm:entry-point')
  Context._();

  //registerShaderLibrary() async

  Future<RasterPipeline> createRasterPipeline({
    required Shader vertex,
    required Shader fragment,
    BlendOptions blendOptions = const BlendOptions(),
    StencilOptions stencilOptions = const StencilOptions(),
    List<VertexAttribute> vertexLayout = const <VertexAttribute>[],
    List<UniformSlot> uniformLayout = const <UniformSlot>[],
  }) async {
    return RasterPipeline();
  }
}

Context getContext() {
  return Context._();
}
