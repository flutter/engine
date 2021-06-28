#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(sign(-72.45) + 1.0, sign(-12.34) + 2.0, 0.0, sign(0.1234));
}
