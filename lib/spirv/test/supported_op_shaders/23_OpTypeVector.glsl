#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    vec2 ones = vec2(a);
    vec3 zeros = vec3(0.0, 0.0, 0.0);
    fragColor = vec4(zeros[0], ones[0], zeros[2], ones[1]);
}
