#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

// For a given incident vector I and surface normal N reflect returns the reflection direction calculated as I - 2.0 * dot(N, I) * N.
void main() {
    // TODO: Setup I and N so that result is [0.0, 1.0]
    fragColor = vec4(
        reflect(vec2(0.99337748344, -0.1655629139), vec2(0.6, 0.8))[0],
        reflect(vec2(0.99337748344, -0.1655629139), vec2(0.6, 0.8))[1],
        0.0,
        1.0
    );
}
