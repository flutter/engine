// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "flutter/lib/ui/painting/image_generator_registry.h"
#include "third_party/skia/include/codec/SkCodec.h"
#include "third_party/skia/include/core/SkImageGenerator.h"
#include "third_party/skia/src/codec/SkCodecImageGenerator.h"
#ifdef OS_MACOSX
#include "third_party/skia/include/ports/SkImageGeneratorCG.h"
#elif OS_WIN
#include "third_party/skia/include/ports/SkImageGeneratorWIC.h"
#endif

namespace flutter {

ImageGeneratorRegistry::ImageGeneratorRegistry() : weak_factory_(this) {
  add(
      [](sk_sp<SkData> buffer) {
        return BuiltinSkiaCodecImageGenerator::makeFromData(buffer);
      },
      0);

#ifdef OS_MACOSX
  add(
      [](sk_sp<SkData> buffer) {
        auto generator = SkImageGeneratorCG::MakeFromEncodedCG(buffer);
        return BuiltinSkiaImageGenerator::makeFromGenerator(
            std::move(generator));
      },
      0);
#elif OS_WIN
  add(
      [](sk_sp<SkData> buffer) {
        auto generator = SkImageGeneratorWIC::MakeFromEncodedWIC(buffer);
        return BuiltinSkiaImageGenerator::makeFromGenerator(
            std::move(generator));
      },
      0);
#endif
}

void ImageGeneratorRegistry::add(ImageGeneratorFactory factory,
                                 int32_t priority) {
  image_generator_factories_.push_back({factory, priority});
  std::stable_sort(image_generator_factories_.begin(),
                   image_generator_factories_.end());
}

std::unique_ptr<ImageGenerator> ImageGeneratorRegistry::createCompatible(
    sk_sp<SkData> buffer) {
  for (auto& factory : image_generator_factories_) {
    std::unique_ptr<ImageGenerator> result = factory.callback(buffer);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

fml::WeakPtr<ImageGeneratorRegistry> ImageGeneratorRegistry::GetWeakPtr()
    const {
  return weak_factory_.GetWeakPtr();
}

}  // namespace flutter
