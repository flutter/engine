#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(
        // cross product of parallel vectors is a zero vector
        cross(vec3(1.0, 2.0, 3.0), vec3(2.0, 4.0, 6.0))[0],
        1.0,
        // cross product of parallel vectors is a zero vector
        cross(vec3(1.0, 2.0, 3.0), vec3(2.0, 4.0, 6.0))[2],
        1.0
    );
}
