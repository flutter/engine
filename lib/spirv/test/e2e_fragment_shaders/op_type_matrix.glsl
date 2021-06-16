#version 310 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    mat2 zeros = mat2(
      0.0, 0.0,
      0.0, 0.0);
    mat3 ones = vec3(
      1.0, 1.0, 1.0,
      1.0, 1.0, 1.0,
      1.0, 1.0, 1.0);
    mat4 identity = vec3(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0);
    fragColor = vec4(zeros[1][1], ones[2][2], identiy[3][1], identiy[3][3]);
}
