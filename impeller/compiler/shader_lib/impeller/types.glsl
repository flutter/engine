// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TYPES_GLSL_
#define TYPES_GLSL_

#extension GL_AMD_gpu_shader_half_float : enable
#extension GL_AMD_gpu_shader_half_float_fetch : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable

#ifdef IMPELLER_TARGET_OPENGLES

// OpenGLES 2 targets GLSL ES 1.0, which doesn't support explicit arithmetic
// types on individual declarations. So for GLES, we provide macros for 16 bit
// types to keep writing cross API shaders easy.
//
// By default, OpenGLES sets the floating point precision to `highp` for vertex
// shaders and `mediump` for fragment shaders. Fragment shader samplers are set
// to `lowp` by default. Individual shaders may explicitly declare the
// precision for all declarations in the shader.
//
// For example:
//  precision mediump sampler2D;
//  precision mediump float;

#define float16_t float
#define float32_t float
#define float64_t float

#define f16vec2 vec2
#define f32vec2 vec2
#define f64vec2 vec2

#define f16vec3 vec3
#define f32vec3 vec3
#define f64vec3 vec3

#define f16vec4 vec4
#define f32vec4 vec4
#define f64vec4 vec4

#define f16mat4 mat4
#define f32mat4 mat4
#define f64mat4 mat4

#define f16sampler2D sampler2D
#define f32sampler2D sampler2D
#define f64sampler2D sampler2D

#define uint8_t uint
#define uint16_t uint
#define uint32_t uint
#define uint64_t uint

#define u8vec2 vec2
#define u16vec2 vec2
#define u32vec2 vec2
#define u64vec2 vec2

#define u8vec3 vec3
#define u16vec3 vec3
#define u32vec3 vec3
#define u64vec3 vec3

#define u8vec4 vec4
#define u16vec4 vec4
#define u32vec4 vec4
#define u64vec4 vec4

#define u8mat4 mat4
#define u16mat4 mat4
#define u32mat4 mat4
#define u64mat4 mat4

#elif defined(IMPELLER_TARGET_VULKAN)

// Vulkan does not support 16 bit floating point samplers.
// https://registry.khronos.org/vulkan/specs/1.1-extensions/html/vkspec.html#VUID-StandaloneSpirv-OpTypeImage-04656
#define f16sampler2D f32sampler2D

#endif

#define BoolF float
#define BoolV2 vec2
#define BoolV3 vec3
#define BoolV4 vec4

#endif
