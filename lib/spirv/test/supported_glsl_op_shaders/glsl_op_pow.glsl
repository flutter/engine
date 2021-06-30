#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        0.0,
        // anything to the 0th power is 1.0
        pow(3.14, 0.0),
        0.0,
        // 3.0^4.0 - 80.0 = 81.0 - 80.0 = 1.0
        pow(3.0, 4.0) - 80.0
    );
}
