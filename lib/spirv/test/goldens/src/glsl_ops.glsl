#version 320 es

precision highp float;

layout ( location = 0 ) out vec4 oColor;

// Use all of the supported glsl operations in one shader.
void main() {
  vec3 a = vec3(5.0);
  vec3 b = vec3(15.0);
  vec3 i13 = abs(a);
  vec3 i14 = acos(i13);
  vec3 i15 = asin(i14);
  vec3 i16 = atan(i15);
  vec3 i17 = ceil(i16);
  vec3 i18 = clamp(i17, a, b);
  vec3 i19 = cos(i18);
  vec3 i20 = degrees(i19);
  vec3 i21 = exp(i20);
  vec3 i22 = exp2(i21);
  vec3 i23 = faceforward(i22, a, b);
  vec3 i24 = floor(i23);
  vec3 i25 = fract(i24);
  vec3 i26 = inversesqrt(i25);
  vec3 i27 = log(i26);
  vec3 i28 = log2(i27);
  vec3 i29 = mix(a, b, i28);
  vec3 i30 = radians(i29);
  vec3 i31 = reflect(i30, a);
  vec3 i32 = sign(i31);
  vec3 i33 = sin(i32);
  vec3 i34 = smoothstep(a, b, i33);
  vec3 i35 = sqrt(i34);
  vec3 i36 = step(a, i35);
  vec3 i37 = tan(i36);
  vec3 i38 = trunc(i37);
  vec3 i39 = atan(i38, b);
  vec3 i40 = pow(i39, a);
  vec3 i41 = min(i40, a);
  vec3 i42 = max(i41, b);
  float d = distance(i42, a);
  oColor = vec4(i40, d);
}
