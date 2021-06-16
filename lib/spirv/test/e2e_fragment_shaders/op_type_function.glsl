#version 310 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(zero(), one(), zero(), one());
}

float zero() {
  return 0.0;
}

float one() {
  return 1.0;
}
