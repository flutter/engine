#version 320 es

precision highp float;

layout ( location = 0 ) out vec4 oColor;

float floatfn(float p) {
  return p;
}

vec2 vec2fn(vec2 p) {
  return p;
}

vec3 vec3fn(vec3 p) {
  return p;
}

vec4 vec4fn(vec4 p) {
  return p;
}

mat2 mat2fn(mat2 p) {
  return p;
}

mat3 mat3fn(mat3 p) {
  return p;
}

mat4 mat4fn(mat4 p) {
  return p;
}

// define and use functions of various types
void main() {
  float f = floatfn(10.0);
  vec2 v2 = vec2fn(vec2(f));
  vec3 v3 = vec3fn(vec3(v2, f));
  vec4 v4 = vec4fn(vec4(v3, f));
  mat2 m2 = mat2fn(mat2(v4.xy, v4.zw));
  mat3 m3 = mat3fn(mat3(m2));
  mat4 m4 = mat4fn(mat4(m3));
  oColor = vec4(1) * m4;
}
