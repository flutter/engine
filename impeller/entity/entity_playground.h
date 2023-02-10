// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/playground/playground_test.h"

namespace impeller {

class EntityPlayground : public PlaygroundTest {
 public:
  using EntityPlaygroundCallback =
      std::function<bool(ContentContext& context, RenderPass& pass)>;

  EntityPlayground();

  ~EntityPlayground();

  bool OpenPlaygroundHere(Entity entity);

  bool OpenPlaygroundHere(EntityPlaygroundCallback callback);

  bool PumpSingleFrame(Entity entity);

  bool PumpSingleFrame(EntityPlaygroundCallback callback);

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(EntityPlayground);
};

inline ::testing::AssertionResult SizesEqual(const std::vector<ISize>& a,
                                             const std::vector<ISize>& b) {
  if (a.size() != b.size()) {
    return ::testing::AssertionFailure()
           << "Size vectors are different lengths.";
  }
  for (auto i = 0u; i < a.size(); i++) {
    if (a[i] != b[i]) {
      return ::testing::AssertionFailure()
             << a[i] << " Is not equal to " << b[i] << ".";
    }
  }

  return ::testing::AssertionSuccess();
}

#define ASSERT_TEXTURE_ALLOCATION_EQ(a, b) ASSERT_PRED2(&SizesEqual, a, b)

}  // namespace impeller
