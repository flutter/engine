// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_REGISTRY_H_
#define FLUTTER_LIB_UI_PAINTING_IMAGE_GENERATOR_REGISTRY_H_

#include <functional>
#include <vector>

#include "flutter/fml/mapping.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/lib/ui/painting/image_generator.h"

namespace flutter {

/// Keeps a priority-ordered registry of image generator builders to be used
/// when decoding images. This object must be created, accessed, and collected
/// on the UI thread (typically the engine or its runtime controller).
class ImageGeneratorRegistry {
 public:
  using ImageGeneratorFactory =
      std::function<std::unique_ptr<ImageGenerator>(sk_sp<SkData> buffer)>;

  ImageGeneratorRegistry();

  ~ImageGeneratorRegistry();

  void AddFactory(ImageGeneratorFactory factory, int32_t priority);

  /// Walks the list of image generator builders in descending priority order
  /// until a compatible SkImageGenerator is able to be built. If no compatible
  /// image generator could be produced, `std::unique_ptr(nullptr)` is returned.
  std::unique_ptr<ImageGenerator> CreateCompatibleGenerator(
      sk_sp<SkData> buffer);

  fml::WeakPtr<ImageGeneratorRegistry> GetWeakPtr() const;

 private:
  struct PrioritizedFactory {
    ImageGeneratorFactory callback;

    int32_t priority = 0;

    // Order by descending priority.
    constexpr bool operator<(const PrioritizedFactory& other) const {
      return priority > other.priority;
    }
  };
  std::vector<PrioritizedFactory> image_generator_factories_;
  fml::WeakPtrFactory<ImageGeneratorRegistry> weak_factory_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_IMAGE_DECODER_H_
