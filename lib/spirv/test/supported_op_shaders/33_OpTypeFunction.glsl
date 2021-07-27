#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

float zero() {
  return 0.0;
}

float one() {
  return a;
}

void main() {
    fragColor = vec4(zero(), one(), zero(), one());
}
