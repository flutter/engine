// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision highp float;

#include <impeller/gaussian.glsl>
#include <impeller/types.glsl>

uniform FragInfo {
  vec4 color;
  vec2 rect_size;
  vec2 corner_radii;
  float blur_sigma;
}
frag_info;

in vec2 v_position;

out vec4 frag_color;

const float kSampleCount = 4.0;

// Compute a fast gaussian integral for two coordinates.
vec4 IPVec2FastGaussianIntegral2(vec4 x, float sigma) {
  return 1.0 / (1.0 + exp(-kSqrtThree / sigma * x));
}

// Compute the gaussian at four sample points.
vec4 IPGaussian4(vec4 x, float sigma) {
  float variance = sigma * sigma;
  return exp(-0.5f * x * x / variance) / (kSqrtTwoPi * sigma);
}

vec4 RRectBlurX(float x, vec4 samples_y, vec2 half_size) {
  // The vertical edge of the rrect consists of a flat portion and a curved
  // portion, the two of which vary in size depending on the size of the
  // corner radii, both adding up to half_size.y.
  // half_size.y - corner_radii.y is the size of the vertical flat
  // portion of the rrect.
  // subtracting the absolute value of the Y sample_position will be
  // negative (and then clamped to 0) for positions that are located
  // vertically in the flat part of the rrect, and will be the relative
  // distance from the center of curvature otherwise.
  vec4 space_y = min(
      vec4(0.0), vec4(half_size.y - frag_info.corner_radii.y) - abs(samples_y));
  // space is now in the range [0.0, corner_radii.y]. If the y sample was
  // in the flat portion of the rrect, it will be 0.0

  // We will now calculate rrect_distance as the distance from the centerline
  // of the rrect towards the near side of the rrect.
  // half_size.x - frag_info.corner_radii.x is the size of the horizontal
  // flat portion of the rrect.
  // We add to that the X size (space_x) of the curved corner measured at
  // the indicated Y coordinate we calculated as space_y, such that:
  //   (space_y / corner_radii.y)^2 + (space_x / corner_radii.x)^2 == 1.0
  // Since we want the space_x, we rearrange the equation as:
  //   space_x = corner_radii.x * sqrt(1.0 - (space_y / corner_radii.y)^2)
  // We need to prevent negative values inside the sqrt which can occur
  // when the Y sample was beyond the vertical size of the rrect and thus
  // space_y was larger than corner_radii.y.
  // The calling function RRectBlur will never provide a Y sample outside
  // of that range, though, so the max(0.0) is mostly a precaution.
  vec4 unit_space_y = space_y / frag_info.corner_radii.y;
  vec4 unit_space_x =
      sqrt(max(vec4(0.0), vec4(1.0) - (unit_space_y * unit_space_y)));
  vec4 rrect_distance =
      half_size.x - frag_info.corner_radii.x * (vec4(1.0) - unit_space_x);

  // Now we integrate the Gaussian over the range of the relative positions
  // of the left and right sides of the rrect relative to the sampling
  // X coordinate.
  vec4 integral_ab =
      IPVec2FastGaussianIntegral2(x + vec4(-rrect_distance.x, rrect_distance.x,
                                           -rrect_distance.y, rrect_distance.y),
                                  frag_info.blur_sigma);
  vec4 integral_cd =
      IPVec2FastGaussianIntegral2(x + vec4(-rrect_distance.z, rrect_distance.z,
                                           -rrect_distance.w, rrect_distance.w),
                                  frag_info.blur_sigma);

  // integral.y/w contains the evaluation of the indefinite gaussian integral
  // function at (X + rrect_distance) and integral.x/z contains the evaluation
  // of it at (X - rrect_distance). Subtracting the two produces the
  // integral result over the range from one to the other.
  return vec4(integral_ab.y - integral_ab.x, integral_ab.w - integral_ab.z,
              integral_cd.y - integral_cd.x, integral_cd.w - integral_cd.z);
}

float RRectBlur(vec2 sample_position, vec2 half_size) {
  // Limit the sampling range to 3 standard deviations in the Y direction from
  // the kernel center to incorporate 99.7% of the color contribution.
  float half_sampling_range = frag_info.blur_sigma * 3.0;

  // We want to cover the range [Y - half_range, Y + half_range], but we
  // don't want to sample beyond the edge of the rrect (where the RRectBlurX
  // function produces bad information and where the real answer at those
  // locations will be 0.0 anyway).
  float begin_y = max(-half_sampling_range, sample_position.y - half_size.y);
  float end_y = min(half_sampling_range, sample_position.y + half_size.y);
  float interval = (end_y - begin_y) / kSampleCount;

  // Sample the X blur 4 times, weighted by the Gaussian function.
  vec4 samples =
      vec4(begin_y, begin_y + interval, fma(2, interval, begin_y), end_y);
  vec4 sample_y = vec4(sample_position.y) - samples;

  vec4 integrations = RRectBlurX(sample_position.x, sample_y, half_size);
  vec4 gaussians = IPGaussian4(samples, frag_info.blur_sigma);

  vec4 result = gaussians * integrations;
  return interval * (result.x + result.y + result.z + result.w);
}

void main() {
  vec2 half_size = frag_info.rect_size * 0.5;
  vec2 sample_position = v_position - half_size;

  frag_color = frag_info.color * RRectBlur(sample_position, half_size);
}
