// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// @dart = 2.12

part of spirv;

// This file contains a subset of SPIR-V constants defined at
// https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html

// Header constants
const int _magicNumber = 0x07230203;

// Supported ExecutionModes
const int _originLowerLeft = 8;

// Supported memory models
const int _addressingModelLogical = 0;
const int _memoryModelGLSL450 = 1;

// Supported capabilities
const int _capabilityMatrix = 0;
const int _capabilityShader = 1;
const int _capabilityLinkage = 2;

// Supported storage classes
const int _storageClassUniformConstant = 0;
const int _storageClassInput = 1;
const int _storageClassOutput = 3;
const int _storageClassFunction = 7;

// Explicity supported decorations, others are ignored
const int _decorationBuiltIn = 11;
const int _decorationLocation = 30;

// Explicitly supported builtin types
const int _builtinFragCoord = 15;

// Supported instructions
const int _opExtInstImport = 11;
const int _opExtInst = 12;
const int _opMemoryModel = 14;
const int _opEntryPoint = 15;
const int _opExecutionMode = 16;
const int _opCapability = 17;
const int _opTypeVoid = 19;
const int _opTypeBool = 20;
const int _opTypeInt = 21;
const int _opTypeFloat = 22;
const int _opTypeVector = 23;
const int _opTypeMatrix = 24;
const int _opTypePointer = 32;
const int _opTypeFunction = 33;
const int _opConstant = 43;
const int _opConstantComposite = 44;
const int _opFunction = 54;
const int _opFunctionParameter = 55;
const int _opFunctionEnd = 56;
const int _opFunctionCall = 57;
const int _opVariable = 59;
const int _opLoad = 61;
const int _opStore = 62;
const int _opAccessChain = 65;
const int _opDecorate = 71;
const int _opVectorShuffle = 79;
const int _opCompositeConstruct = 80;
const int _opCompositeExtract = 81;
const int _opFNegate = 127;
const int _opFAdd = 129;
const int _opFSub = 131;
const int _opFMul = 133;
const int _opFDiv = 136;
const int _opFMod = 141;
const int _opVectorTimesScalar = 142;
const int _opMatrixTimesScalar = 143;
const int _opVectorTimesMatrix = 144;
const int _opMatrixTimesVector = 145;
const int _opMatrixTimesMatrix = 146;
const int _opDot = 148;
const int _opLabel = 248;
const int _opReturn = 253;
const int _opReturnValue = 254;

// GLSL extension constants defined at
// https://www.khronos.org/registry/spir-v/specs/unified1/GLSL.std.450.html

// Supported GLSL extension name
const String _glslStd450 = 'GLSL.std.450';

// Supported GLSL ops
const int _glslStd450Trunc = 3;
const int _glslStd450FAbs = 4;
const int _glslStd450FSign = 6;
const int _glslStd450Floor = 8;
const int _glslStd450Ceil = 9;
const int _glslStd450Fract = 10;
const int _glslStd450Radians = 11;
const int _glslStd450Degrees = 12;
const int _glslStd450Sin = 13;
const int _glslStd450Cos = 14;
const int _glslStd450Tan = 15;
const int _glslStd450Asin = 16;
const int _glslStd450Acos = 17;
const int _glslStd450Atan = 18;
const int _glslStd450Atan2 = 25;
const int _glslStd450Pow = 26;
const int _glslStd450Exp = 27;
const int _glslStd450Log = 28;
const int _glslStd450Exp2 = 29;
const int _glslStd450Log2 = 30;
const int _glslStd450Sqrt = 31;
const int _glslStd450InverseSqrt = 32;
const int _glslStd450FMin = 37;
const int _glslStd450FMax = 40;
const int _glslStd450FClamp = 43;
const int _glslStd450FMix = 46;
const int _glslStd450Step = 48;
const int _glslStd450SmoothStep = 49;
const int _glslStd450Length = 66;
const int _glslStd450Distance = 67;
const int _glslStd450Cross = 68;
const int _glslStd450Normalize = 69;
const int _glslStd450FaceForward = 70;
const int _glslStd450Reflect = 71;

