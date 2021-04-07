#version 320 es

precision highp float;

layout ( location = 0 ) out vec4 oColor;

layout ( location = 0 ) uniform float iTime;
layout ( location = 1 ) uniform vec2 iResolution;

vec2 helper(float p) {
  float holla = p+3.0+iTime;
  return vec2(holla+5.0);
}

void main() {
  oColor = vec4(helper(8.0)*iResolution, gl_FragCoord.xy);
}
