// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

uniform FragInfo {
  vec4 color;
  float blur_radius;
  vec2 rect_size;
  float corner_radius;
}
frag_info;

in vec2 v_position;

out vec4 frag_color;

// Simple logistic sigmoid with a domain of [-1, 1] and range of [0, 1].
float Sigmoid(float x) {
  return 1.03731472073 / (1 + exp(-4 * x)) - 0.0186573603638;
}

float CircleDistance(vec2 sample_position,
                     vec2 sphere_position,
                     float sphere_size) {
  return length(sample_position - sphere_position) - sphere_size;
}

float RectDistance(vec2 sample_position, vec2 rect_size) {
  vec2 space = abs(sample_position) - rect_size;
  return length(max(space, 0.0)) + min(max(space.x, space.y), 0.0);
}

float RRectDistance(vec2 sample_position, vec2 rect_size, float corner_radius) {
  vec2 space = abs(sample_position) - rect_size + corner_radius;
  return length(max(space, 0.0)) + min(max(space.x, space.y), 0.0) -
         corner_radius;
}

float SquircleDistance(vec2 sample_position, float n) {
  return pow(pow(abs(sample_position.x), n) + pow(abs(sample_position.y), n),
             1.0 / n) -
         1.0;
}

void main() {
  float dist = RRectDistance(v_position - frag_info.rect_size / 2.0,
                             frag_info.rect_size, frag_info.corner_radius);
  float shadow_mask =
      Sigmoid(clamp(1.0 - length(dist / frag_info.corner_radius), 0.0, 1.0));
  frag_color = frag_info.color * shadow_mask;
}
