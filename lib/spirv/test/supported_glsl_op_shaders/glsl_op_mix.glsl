#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        // the start of the range 0.0 to 1.0 is 0.0
        mix(0.0, 1.0, 0.0),
        // 1/5 the way between 0.0 and 5.0 is 1.0
        mix(0.0, 5.0, 0.2),
        // half-way between 3.0 and 7.0 is 5.0, subtract 5.0 to get 0.0
        mix(3.0, 7.0, 0.5) - 5.0,
        // 3/4 the way between -0.5 and 1.5 is 1.0
        mix(-0.5, 1.5, 0.75)
    );
}
