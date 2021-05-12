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

/// @brief Keeps a priority-ordered registry of image generator builders to be
///        used when decoding images. This object must be created, accessed, and
///        collected on the UI thread (typically the engine or its runtime
///        controller).
class ImageGeneratorRegistry {
 public:
  using ImageGeneratorFactory =
      std::function<std::unique_ptr<ImageGenerator>(sk_sp<SkData> buffer)>;

  ImageGeneratorRegistry();

  ~ImageGeneratorRegistry();

  /// @brief      Install a new factory for image generators
  /// @param[in]  factory   Callback that produces `ImageGenerator`s for
  ///                       compatible input data.
  /// @param[in]  priority  The priority used to determine the order in which
  ///                       factories are tried. Higher values mean higher
  ///                       priority. The built-in Skia codecs are installed at
  ///                       priority 0.
  /// @see        `CreateCompatibleGenerator`
  void AddFactory(ImageGeneratorFactory factory, int32_t priority);

  /// @brief      Walks the list of image generator builders in descending
  ///             priority order until a compatible `ImageGenerator` is able to
  ///             be built. This method is safe to perform on the UI thread, as
  ///             checking for `ImageGenerator` compatibility is expected to be
  ///             a lightweight operation. The returned `ImageGenerator` can
  ///             then be used to fully decode the image on e.g. the IO thread.
  /// @param[in]  buffer  The raw encoded image data.
  /// @return     An `ImageGenerator` that is compatible with the input buffer.
  ///             If no compatible `ImageGenerator` type was found, then
  ///             `std::unique_ptr<ImageGenerator>(nullptr)` is returned.
  /// @see        `ImageGenerator`
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
