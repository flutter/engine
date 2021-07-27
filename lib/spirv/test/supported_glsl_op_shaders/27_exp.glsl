#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(
        0.0,
        // e^0.0 = 1.0
        exp(a * 0.0),
        0.0,
        // e^2.0 - 6.38905609893 = 7.38905609893 - 6.38905609893 = 1.0
        exp(a * 2.0) - 6.38905609893
    );
}
