// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "flutter/testing/testing.h"
#include "gtest/gtest.h"
#include "impeller/entity/entity_pass_target.h"
#include "impeller/entity/entity_playground.h"

namespace impeller {
namespace testing {

using EntityPassTargetTest = EntityPlayground;
INSTANTIATE_PLAYGROUND_SUITE(EntityPassTargetTest);

TEST_P(EntityPassTargetTest, SwapWithMSAATexture) {
  if (GetContentContext()
          ->GetDeviceCapabilities()
          .SupportsImplicitResolvingMSAA()) {
    GTEST_SKIP() << "Implicit MSAA is used on this device.";
  }
  auto content_context = GetContentContext();
  auto buffer = content_context->GetContext()->CreateCommandBuffer();
  auto render_target = RenderTarget::CreateOffscreenMSAA(
      *content_context->GetContext(),
      *GetContentContext()->GetRenderTargetCache(), {100, 100});

  auto entity_pass_target = EntityPassTarget(render_target, false);

  auto color0 = entity_pass_target.GetRenderTarget()
                    .GetColorAttachments()
                    .find(0u)
                    ->second;
  auto msaa_tex = color0.texture;
  auto resolve_tex = color0.resolve_texture;

  entity_pass_target.Flip(
      *content_context->GetContext()->GetResourceAllocator());

  color0 = entity_pass_target.GetRenderTarget()
               .GetColorAttachments()
               .find(0u)
               ->second;

  ASSERT_EQ(msaa_tex, color0.texture);
  ASSERT_NE(resolve_tex, color0.resolve_texture);
}

TEST_P(EntityPassTargetTest, SwapWithMSAAImplicitResolve) {
  if (!GetContentContext()
           ->GetDeviceCapabilities()
           .SupportsImplicitResolvingMSAA()) {
    GTEST_SKIP() << "Implicit MSAA is not used on this device.";
  }
  auto content_context = GetContentContext();
  auto buffer = content_context->GetContext()->CreateCommandBuffer();
  auto render_target = RenderTarget::CreateOffscreenMSAA(
      *content_context->GetContext(),
      *GetContentContext()->GetRenderTargetCache(), {100, 100});

  auto entity_pass_target = EntityPassTarget(render_target, false);

  auto color0 = entity_pass_target.GetRenderTarget()
                    .GetColorAttachments()
                    .find(0u)
                    ->second;
  auto msaa_tex = color0.texture;
  auto resolve_tex = color0.resolve_texture;

  ASSERT_EQ(msaa_tex, resolve_tex);

  entity_pass_target.Flip(
      *content_context->GetContext()->GetResourceAllocator());

  color0 = entity_pass_target.GetRenderTarget()
               .GetColorAttachments()
               .find(0u)
               ->second;

  ASSERT_EQ(msaa_tex, color0.texture);
  ASSERT_NE(resolve_tex, color0.resolve_texture);
}

}  // namespace testing
}  // namespace impeller
