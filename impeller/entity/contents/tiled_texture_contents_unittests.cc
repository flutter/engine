// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "impeller/core/sampler_descriptor.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/tiled_texture_contents.h"
#include "impeller/renderer/capabilities.h"
#include "impeller/renderer/sampler_library.h"
#include "impeller/renderer/testing/mocks.h"
#include "impeller/typographer/backends/skia/typographer_context_skia.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace impeller {
namespace testing {

TEST(TiledTextureContentsTest, RenderAppendsExpectedCommands) {
  auto context = std::make_shared<MockImpellerContext>();

  auto sampler_library = std::make_shared<MockSamplerLibrary>();
  SamplerDescriptor sampler_desc;
  auto sampler = std::make_shared<MockSampler>(sampler_desc);
  EXPECT_CALL(*sampler_library, GetSampler)
      .WillRepeatedly(::testing::Return(sampler));

  EXPECT_CALL(*context, GetSamplerLibrary)
      .WillRepeatedly(::testing::Return(sampler_library));

  const std::shared_ptr<const Capabilities> capabilities =
      std::make_shared<const MockCapabilities>();
  EXPECT_CALL(*context, GetCapabilities)
      .WillRepeatedly(::testing::ReturnRef(capabilities));

  auto content_context =
      ContentContext(context, TypographerContextSkia::Make());
  auto render_pass = MockRenderPass(context, {});

  TextureDescriptor texture_desc;
  auto texture = std::make_shared<MockTexture>(texture_desc);
  EXPECT_CALL(*texture, GetSize).WillOnce(::testing::Return(ISize(100, 100)));

  TiledTextureContents contents;
  contents.SetTexture(texture);
  contents.SetGeometry(Geometry::MakeCover());

  ASSERT_TRUE(contents.Render(content_context, {}, render_pass));
  const std::vector<Command>& commands = render_pass.GetCommands();

  ASSERT_EQ(commands.size(), 1u);
  ASSERT_STREQ(commands[0].pipeline->GetDescriptor().GetLabel().c_str(),
               "Something");
}

}  // namespace testing
}  // namespace impeller
