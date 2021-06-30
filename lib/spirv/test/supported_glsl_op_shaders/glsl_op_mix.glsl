#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        mix(0.0, 1.0, 0.0),
        mix(0.0, 5.0, 0.2),
        mix(3.0, 7.0, 0.5) - 5.0,
        mix(-0.5, 1.5, 0.75)
    );
}
