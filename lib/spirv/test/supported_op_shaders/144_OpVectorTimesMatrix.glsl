#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    mat4 identity = mat4(a);
    fragColor = vec4(0.0, 1.0, 0.0, 1.0) * identity;
}
