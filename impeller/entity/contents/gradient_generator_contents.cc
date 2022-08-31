// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "gradient_generator_contents.h"

#include "flutter/fml/logging.h"
#include "flutter/impeller/renderer/context.h"
#include "flutter/impeller/renderer/texture.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

static void AppendColor(const Color& color, std::vector<uint8_t>* colors) {
  auto converted = color.Premultiply().ToR8G8B8A8();
  colors->push_back(converted[0]);
  colors->push_back(converted[1]);
  colors->push_back(converted[2]);
  colors->push_back(converted[3]);
}

std::shared_ptr<Texture> CreateGradientTexture(
    const std::vector<Color>& colors,
    const std::vector<Scalar>& stops,
    std::shared_ptr<impeller::Context> context) {
  Scalar scale;
  if (stops.size() == 2) {
    scale = 2.0;
  } else {
    auto minimum_delta = 1.0;
    for (size_t i = 1; i < stops.size(); i++) {
      auto value = stops[i] - stops[i - 1];
      if (value < minimum_delta) {
        minimum_delta = value;
      }
    }
    // Avoid creating textures that are absurdly large due to stops that are
    // very close together.
    scale = std::min(std::ceil(1.0 / minimum_delta), 1024.0);
  }

  // If the computed scale is nearly the same as the color length, then the
  // stops are evenly spaced and we can lerp entirely in the gradient shader.
  // Thus we only need to populate a texture with all of the colors in orer. If
  // the stop is a value greater than one, then the stops aren't evenly spaced.
  // In this case, we attempt to scale the texture so that the smallest stop
  // gets at least one index. Then it follows that we must compute lerped values
  // for each sample. In the event that some colors do not get a dedicated slot,
  // due to hitting the artifically capped texture size of 1024
  // - these results are currently incorrect.
  u_int32_t width = std::round(scale);
  impeller::TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = impeller::StorageMode::kHostVisible;
  texture_descriptor.format = PixelFormat::kR8G8B8A8UNormInt;
  texture_descriptor.size = {width, 1};
  auto texture =
      context->GetResourceAllocator()->CreateTexture(texture_descriptor);
  if (!texture) {
    FML_DLOG(ERROR) << "Could not create Impeller texture.";
    return nullptr;
  }
  std::vector<uint8_t> color_stop_channels;
  color_stop_channels.reserve(width * 4);

  if (ScalarNearlyEqual(scale, colors.size()) && colors.size() <= 1024) {
    for (auto i = 0u; i < colors.size(); i++) {
      AppendColor(colors[i], &color_stop_channels);
    }
  } else {
    Color previous_color = colors[0];
    auto previous_stop = 0.0;
    auto previous_color_index = 0;

    // The first index is always equal to the first color, exactly.
    AppendColor(previous_color, &color_stop_channels);

    for (auto i = 1u; i < width - 1; i++) {
      // We're almost exactly equal to the next stop.
      auto scaled_i = i / scale;
      Color next_color = colors[previous_color_index + 1];
      auto next_stop = stops[previous_color_index + 1];
      if (ScalarNearlyEqual(scaled_i, next_stop)) {
        AppendColor(next_color, &color_stop_channels);

        previous_color = next_color;
        previous_stop = next_stop;
        previous_color_index += 1;
      } else if (scaled_i < next_stop) {
        // We're still between the current stop and the next stop.
        auto t = (scaled_i - previous_stop) / (next_stop - previous_stop);
        auto mixed_color = Color::lerp(previous_color, next_color, t);

        AppendColor(mixed_color, &color_stop_channels);
      } else {
        // We've slightly overshot the next stop. In theory this only happens if
        // we have scaled our texture such that not every stop gets their own
        // index. For now I am simply ignoring the inbetween colors. Currently
        // this requires a gradient with either an absurd number of textures
        size_t j = 2;
        while (scaled_i > next_stop) {
          next_stop = stops[j];
          j++;
        }
        next_color = colors[previous_color_index + j];
        auto t = (scaled_i - previous_stop) / (next_stop - previous_stop);
        auto mixed_color = Color::lerp(previous_color, next_color, t);
        AppendColor(mixed_color, &color_stop_channels);

        previous_color = next_color;
        previous_stop = next_stop;
        previous_color_index += j;
      }
    }
    // The last index is always equal to the last color, exactly.
    AppendColor(colors.back(), &color_stop_channels);
  }

  auto mapping = std::make_shared<fml::DataMapping>(color_stop_channels);
  if (!texture->SetContents(mapping)) {
    FML_DLOG(ERROR) << "Could not copy contents into Impeller texture.";
    return nullptr;
  }
  texture->SetLabel(impeller::SPrintF("Gradient(%p)", texture.get()).c_str());
  return texture;
}

}  // namespace impeller
