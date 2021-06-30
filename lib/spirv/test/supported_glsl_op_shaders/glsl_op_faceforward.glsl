#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        0.0,
        // dot product of incident vector (2nd param) and reference vector (3rd param) is non-negative,
        // so the negated first param is returned, and its first value is 1.0.
        faceforward(vec2(-1.0, 5.0), vec2(1.0, 2.0), vec2(3.0, 4.0))[0],
        0.0,
        // dot product of incident vector (2nd param) and reference vector (3rd param) is negative,
        // so the original first param is returned, and its second value is 5.0, so subtract 4.0 to get 1.0.
        faceforward(vec2(1.0, 5.0), vec2(1.0, -2.0), vec2(3.0, 4.0))[1] - 4.0
    );
}
