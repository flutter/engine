// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_SHADOW_CACHE_H_
#define FLUTTER_IMPELLER_ENTITY_SHADOW_CACHE_H_

#include "impeller/core/texture.h"
#include "impeller/renderer/snapshot.h"

namespace impeller {

struct ShadowKey {
  const Rect rect;
  const Size corner_radii;
  const Color rrect_color;
  const Scalar sigma;
};

}  // namespace impeller

template <>
struct std::hash<impeller::ShadowKey> {
  constexpr std::size_t operator()(const impeller::ShadowKey& sk) const {
    return fml::HashCombine(sk.rect.GetX(), sk.rect.GetY(), sk.rect.GetWidth(),
                            sk.rect.GetHeight(), sk.corner_radii.width,
                            sk.corner_radii.height, sk.rrect_color.ToARGB(),
                            sk.sigma);
  }
};

template <>
struct std::equal_to<impeller::ShadowKey> {
  constexpr bool operator()(const impeller::ShadowKey& lhs,
                            const impeller::ShadowKey& rhs) const {
    return lhs.rect == rhs.rect && lhs.corner_radii == rhs.corner_radii &&
           lhs.rrect_color == rhs.rrect_color && lhs.sigma == rhs.sigma;
  }
};

namespace impeller {

struct ShadowCacheData {
  Snapshot input_snapshot;
  std::shared_ptr<Texture> texture;
  Matrix transform;
};

class ShadowCache {
 public:
  ShadowCache() {}

  ~ShadowCache() = default;

  void StoreData(const ShadowKey& key,
                 Snapshot input_snapshot,
                 std::shared_ptr<Texture> texture,
                 const Matrix& matrix) {
    cache_[key] = ShadowCacheData{input_snapshot, texture, matrix};
  }

  std::optional<ShadowCacheData> GetData(const ShadowKey& key) {
    auto result = cache_.find(key);
    if (result == cache_.end()) {
      return std::nullopt;
    }
    return result->second;
  }

  void End() { cache_.clear(); }

 private:
  std::unordered_map<ShadowKey, ShadowCacheData> cache_;
  Matrix matrix_;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_SHADOW_CACHE_H_
