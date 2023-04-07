// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:ffi';
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart' as ui;

abstract class SkwasmShader implements ui.Shader {
  ShaderHandle get handle;

  @override
  bool get debugDisposed => handle == nullptr;
  
  @override
  void dispose() {
    if (handle != nullptr) {
      shaderDispose(handle);
    }
  }
}

class SkwasmGradient extends SkwasmShader implements ui.Gradient {
  factory SkwasmGradient.linear({
    required ui.Offset from,
    required ui.Offset to,
    required List<ui.Color> colors,
    List<double>? colorStops,
    ui.TileMode tileMode = ui.TileMode.clamp,
    Float32List? matrix4,
  }) => withStackScope((StackScope scope) {
    final RawPointArray endPoints = 
      scope.convertPointArrayToNative(<ui.Offset>[from, to]);
    final RawColorArray nativeColors = scope.convertColorArrayToNative(colors);
    final Pointer<Float> stops = colorStops != null
      ? scope.convertDoublesToNative(colorStops)
      : nullptr;
    final Pointer<Float> matrix = matrix4 != null
      ? scope.convertMatrix4toSkMatrix(matrix4)
      : nullptr;
    final ShaderHandle handle = shaderCreateLinearGradient(
      endPoints,
      nativeColors,
      stops,
      colors.length,
      tileMode.index,
      matrix
    );
    return SkwasmGradient._(handle);
  });

  SkwasmGradient._(this.handle);

  @override
  ShaderHandle handle;
  
  @override
  bool get debugDisposed => handle == nullptr;
  
  @override
  void dispose() {
    super.dispose();
    handle = nullptr;
  }
}

class SkwasmFragmentProgram implements ui.FragmentProgram {
  SkwasmFragmentProgram._(this.name, this.handle);
  factory SkwasmFragmentProgram.fromBytes(String name, Uint8List bytes) {
    final ShaderData shaderData = ShaderData.fromBytes(bytes);

    // TODO(jacksongardner): Can we avoid this copy?
    final List<int> sourceData = utf8.encode(shaderData.source);
    final SkStringHandle sourceString = shaderSourceAllocate(sourceData.length);
    final Pointer<Int8> sourceBuffer = shaderSourceGetData(sourceString);
    int i = 0;
    for (final int byte in sourceData) {
      sourceBuffer[i] = byte;
      i++;
    }
    final RuntimeEffectHandle handle = runtimeEffectCreate(sourceString);
    shaderSourceFree(sourceString);
    return SkwasmFragmentProgram._(name, handle);
  }

  RuntimeEffectHandle handle;
  String name;

  @override
  ui.FragmentShader fragmentShader() =>
    SkwasmFragmentShader(this);

  int get uniformSize => runtimeEffectGetUniformSize(handle);

  void dispose() {
    runtimeEffectDispose(handle);
  }
}

class SkwasmFragmentShader extends SkwasmShader implements ui.FragmentShader {
  SkwasmFragmentShader(
    SkwasmFragmentProgram program, {
    List<SkwasmShader>? childShaders,
  }) : _program = program, 
       _uniformData = dataCreate(program.uniformSize),
       _childShaders = childShaders;

  @override
  ShaderHandle get handle {
    if (_handle == nullptr) {
      _handle = withStackScope((StackScope s) {
        Pointer<ShaderHandle> childShaders = nullptr;
        final int childCount = _childShaders != null ? _childShaders!.length : 0;
        if (childCount != 0) {
          childShaders = s.allocPointerArray(childCount)
            .cast<ShaderHandle>();
          int i = 0;
          for (final SkwasmShader shader in _childShaders!) {
            childShaders[i] = shader.handle;
            i++;
          }
        }
        return shaderCreateRuntimeEffectShader(
          _program.handle,
          _uniformData,
          childShaders,
          childCount,
        );
      });
      _uniformData = nullptr;
    }
    return _handle;
  }

  ShaderHandle _handle = nullptr;
  final SkwasmFragmentProgram _program;
  SkDataHandle _uniformData;
  final List<SkwasmShader>? _childShaders;

  @override
  void setFloat(int index, double value) {
    // We should not set any values after creating the shader.
    assert(_handle == nullptr);
    final Pointer<Float> dataPointer = dataGetPointer(_uniformData).cast<Float>();
    dataPointer[index] = value;
  }

  @override
  void setImageSampler(int index, ui.Image image) {
    // We should not set any values after creating the shader.
    assert(_handle == nullptr);
    // TODO(jacksongardner): implement this when images are implemented
  }
}
