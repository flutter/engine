// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision highp float;

#include <impeller/gaussian.glsl>
#include <impeller/types.glsl>

uniform FragInfo {
  f16vec4 color;
  f16vec2 half_rect_size;
  f16vec2 corner_radii;
  float16_t blur_sigma;
  float16_t half_inv_variance;
  float16_t gaussian_denominator;
  float16_t sqrt_three_over_sigma;
}
frag_info;

in vec2 v_position;

out f16vec4 frag_color;

// Compute a fast gaussian integral for two coordinates.
f16vec4 IPVec2FastGaussianIntegral2(f16vec4 x, float16_t sigma) {
  return 1.0hf / (1.0hf + exp(float16_t(frag_info.sqrt_three_over_sigma) * x));
}

// Compute the gaussian at four sample points.
f16vec4 IPGaussian4(f16vec4 x) {
  return exp(frag_info.half_inv_variance * x * x) *
         frag_info.gaussian_denominator;
}

f16vec4 RRectBlurX(float16_t x, f16vec4 samples_y, f16vec2 half_size) {
  // The vertical edge of the rrect consists of a flat portion and a curved
  // portion, the two of which vary in size depending on the size of the
  // corner radii, both adding up to half_size.y.
  // half_size.y - corner_radii.y is the size of the vertical flat
  // portion of the rrect.
  // subtracting the absolute value of the Y sample_position will be
  // negative (and then clamped to 0) for positions that are located
  // vertically in the flat part of the rrect, and will be the relative
  // distance from the center of curvature otherwise.
  f16vec4 space_y =
      min(f16vec4(0.0hf),
          f16vec4(half_size.y - frag_info.corner_radii.y) - abs(samples_y));
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
  f16vec4 unit_space_y = space_y / frag_info.corner_radii.y;
  f16vec4 unit_space_x =
      sqrt(max(f16vec4(0.0hf), f16vec4(1.0hf) - (unit_space_y * unit_space_y)));
  f16vec4 rrect_distance =
      half_size.x - frag_info.corner_radii.x * (f16vec4(1.0hf) - unit_space_x);

  // Now we integrate the Gaussian over the range of the relative positions
  // of the left and right sides of the rrect relative to the sampling
  // X coordinate.
  f16vec4 integral_ab = IPVec2FastGaussianIntegral2(
      x + f16vec4(-rrect_distance.x, rrect_distance.x, -rrect_distance.y,
                  rrect_distance.y),
      frag_info.blur_sigma);
  f16vec4 integral_cd = IPVec2FastGaussianIntegral2(
      x + f16vec4(-rrect_distance.z, rrect_distance.z, -rrect_distance.w,
                  rrect_distance.w),
      frag_info.blur_sigma);

  // integral.yw contains the evaluation of the indefinite gaussian integral
  // function at (X + rrect_distance) and integral.xz contains the evaluation
  // of it at (X - rrect_distance). Subtracting the two produces the
  // integral result over the range from one to the other.
  return f16vec4(integral_ab.yw - integral_ab.xz,
                 integral_cd.yw - integral_cd.xz);
}

float16_t RRectBlur(f16vec2 sample_position, f16vec2 half_size) {
  // Limit the sampling range to 3 standard deviations in the Y direction from
  // the kernel center to incorporate 99.7% of the color contribution.
  float16_t half_sampling_range = frag_info.blur_sigma * 3.0hf;

  // We want to cover the range [Y - half_range, Y + half_range], but we
  // don't want to sample beyond the edge of the rrect (where the RRectBlurX
  // function produces bad information and where the real answer at those
  // locations will be 0.0 anyway).
  float16_t begin_y =
      max(-half_sampling_range, sample_position.y - half_size.y);
  float16_t end_y = min(half_sampling_range, sample_position.y + half_size.y);
  float16_t interval = (end_y - begin_y) * 0.25hf;  // 1 / 4

  // Sample the X blur 4 times, weighted by the Gaussian function.
  f16vec4 samples =
      f16vec4(begin_y, begin_y + interval, end_y - interval, end_y);
  f16vec4 sample_y = f16vec4(sample_position.y) - samples;

  f16vec4 integrations = RRectBlurX(sample_position.x, sample_y, half_size);
  f16vec4 gaussians = IPGaussian4(samples);

  f16vec4 result = gaussians * integrations;
  return interval * (result.x + result.y + result.z + result.w);
}

void main() {
  f16vec2 half_size = frag_info.half_rect_size;
  f16vec2 sample_position = f16vec2(v_position) - half_size;

  frag_color = frag_info.color * RRectBlur(sample_position, half_size);
}
