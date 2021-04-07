// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// @dart = 2.12

part of spirv;

enum _Type {
  _void,
  _bool,
  _int, 
  float,
  float2,
  float3,
  float4,
  float2x2,
  float3x3,
  float4x4, 
}

class _FunctionType {
  /// Result-id of the return type.
  final int returnType;
  /// Type-id for each parameter.
  final List<int> params;

  _FunctionType(this.returnType, this.params);
}

String _typeName(_Type t, TargetLanguage target) {
  switch (target) {
    case TargetLanguage.sksl:
      return _skslTypeName(t);
    default:
      return _glslTypeName(t);
  }
}

String _skslTypeName(_Type t) {
  switch (t) {
    case _Type._void:
      return 'void';
    case _Type._bool:
      return 'bool';
    case _Type._int:
      return 'int';
    case _Type.float:
      return 'float';
    case _Type.float2:
      return 'float2';
    case _Type.float3:
      return 'float3';
    case _Type.float4:
      return 'float4';
    case _Type.float2x2:
      return 'float2x2';
    case _Type.float3x3:
      return 'float3x3';
    case _Type.float4x4:
      return 'float4x4';
    default:
      throw Error();
  }
}

String _glslTypeName(_Type t) {
  switch (t) {
    case _Type._void:
      return 'void';
    case _Type._bool:
      return 'bool';
    case _Type._int:
      return 'int';
    case _Type.float:
      return 'float';
    case _Type.float2:
      return 'vec2';
    case _Type.float3:
      return 'vec3';
    case _Type.float4:
      return 'vec4';
    case _Type.float2x2:
      return 'mat2';
    case _Type.float3x3:
      return 'mat3';
    case _Type.float4x4:
      return 'mat4';
    default:
      throw Error();
  }
}

int _typeFloatCount(_Type t) {
  switch (t) {
    case _Type.float:
      return 1;
    case _Type.float2:
      return 2;
    case _Type.float3:
      return 3;
    case _Type.float4:
      return 4;
    case _Type.float2x2:
      return 4;
    case _Type.float3x3:
      return 9;
    case _Type.float4x4:
      return 16;
    default:
      throw Error();
  }
}
