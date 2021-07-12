#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        // clamp takes the min (second param, which is 0.0) if x (first param) is less than the lower bound
        clamp(-1.0, 0.0, 1.0),
        // clamp takes the max (third param, which is 1.0) if x (first param) is greater than the upper bound
        clamp(1.5, 0.0, 1.0),
        // clamp takes x (first param, which is 0.0) if x in bounds
        clamp(0.0, -1.0, 1.0),
        // clamp takes x (first param, which is 1.0) if x in bounds
        clamp(1.0, 0.0, 1.0)
    );
}
