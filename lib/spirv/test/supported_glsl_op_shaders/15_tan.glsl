#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        // tan(0.0) = 0.0
        tan(0.0),
        // tan(1.57079632679) = tan(pi / 4.0) = 1.0
        tan(a * 0.785398163),
        0.0,
        1.0
    );
}
