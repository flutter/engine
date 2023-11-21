#version 320 es

layout(location = 0) uniform mat4 mvp;
layout(location = 1) uniform vec4 color;

in vec2 position;
out vec4 v_color;

void main() {
  v_color = color;
  gl_Position = mvp * vec4(position, 0.0, 1.0);
}
