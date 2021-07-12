#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        // e^0.0 = 1.0
        log(1.0),
        // e^1.0 = e
        log(2.718281828459),
        0.0,
        1.0
    );
}
