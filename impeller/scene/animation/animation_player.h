// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

#include "flutter/fml/hash_combine.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/time/time_delta.h"
#include "impeller/geometry/matrix.h"
#include "impeller/scene/animation/animation_clip.h"

namespace impeller {
namespace scene {

class Node;

class AnimationPlayer final {
 public:
  AnimationPlayer();
  ~AnimationPlayer();

  AnimationPlayer(AnimationPlayer&&);
  AnimationPlayer& operator=(AnimationPlayer&&);

  AnimationClip& AddAnimation(std::shared_ptr<Animation> animation,
                              Node* bind_target);

  /// @brief  Advanced all clips and updates animated properties in the scene.
  void Update();

  /// @brief  Reset all bound animation target transforms.
  void Reset();

 private:
  struct DefaultTransform {
    Node* node;
    Matrix transform;

    struct Hash {
      std::size_t operator()(const DefaultTransform& o) const {
        return fml::HashCombine(o.node);
      }
    };

    struct Equal {
      bool operator()(const DefaultTransform& lhs,
                      const DefaultTransform& rhs) const {
        return lhs.node == rhs.node;
      }
    };
  };

  std::unordered_set<DefaultTransform,
                     DefaultTransform::Hash,
                     DefaultTransform::Equal>
      default_target_transforms_;

  std::vector<AnimationClip> clips_;

  std::optional<fml::TimeDelta> previous_time_;

  FML_DISALLOW_COPY_AND_ASSIGN(AnimationPlayer);
};

}  // namespace scene
}  // namespace impeller
