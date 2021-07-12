#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

// 0.0 is returned if x (second param) < edge (first param), and 1.0 is returned otherwise.
void main() {
    fragColor = vec4(
        step(5.0, 4.0),
        step(4.0, 5.0),
        step(-4.0, -5.0),
        step(-5.0, -4.0)
    );
}
