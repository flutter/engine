#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    mat2 zeros = mat2(0.0);
    mat3 ones = mat3(a);
    mat4 identity = mat4(
      a, 0.0, 0.0, 0.0,
      0.0, a, 0.0, 0.0,
      0.0, 0.0, a, 0.0,
      0.0, 0.0, 0.0, a);
    fragColor = vec4(zeros[1][1], ones[2][2], identity[3][1], identity[3][3]);
}
