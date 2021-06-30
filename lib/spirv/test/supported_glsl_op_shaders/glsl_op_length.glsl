#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        length(vec3(0.0, 0.0, 0.0)),
        length(vec2(3.0, 4.0)) - 4.0,
        0.0,
        length(vec4(4.0, -4.0, -4.0, 4.0)) - 7.0
    );
}
