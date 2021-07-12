#version 320 es

precision highp float;

layout ( location = 0 ) out vec4 oColor;

layout ( location = 0 ) uniform float iFloatUniform;
layout ( location = 1 ) uniform vec2 iVec2Uniform;
layout ( location = 2 ) uniform mat2 iMat2Uniform;

void main() {
  oColor = vec4(iFloatUniform, iVec2Uniform, iMat2Uniform[1][1]);
}
