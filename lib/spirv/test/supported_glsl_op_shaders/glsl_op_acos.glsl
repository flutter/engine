#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(acos(1.0), acos(0.54030230586), 0.0, 1.0);
}
