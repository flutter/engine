// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/blit_pass.h"
#include <memory>

#include "impeller/base/strings.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/blit_command.h"
#include "impeller/renderer/host_buffer.h"

namespace impeller {

BlitPass::BlitPass() : transients_buffer_(HostBuffer::Create()) {}

BlitPass::~BlitPass() = default;

HostBuffer& BlitPass::GetTransientsBuffer() {
  return *transients_buffer_;
}

void BlitPass::SetLabel(std::string label) {
  if (label.empty()) {
    return;
  }
  transients_buffer_->SetLabel(SPrintF("%s Transients", label.c_str()));
  OnSetLabel(std::move(label));
}

bool BlitPass::AddCopy(std::shared_ptr<Texture> source,
                       std::shared_ptr<Texture> destination,
                       std::optional<IRect> source_region,
                       IPoint destination_origin,
                       std::string label) {
  if (!source) {
    VALIDATION_LOG << "Attempted to add an invalid copy with no source.";
    return false;
  }
  if (!destination) {
    VALIDATION_LOG << "Attempted to add an invalid copy with no destination.";
    return false;
  }

  commands_.emplace_back(BlitCommand{
      .label = label,
      .data =
          BlitCommand::CopyTextureToTexture{
              .source = source,
              .destination = destination,
              .source_region = source_region.has_value()
                                   ? source_region.value()
                                   : IRect::MakeSize(source->GetSize()),
              .destination_origin = destination_origin,
          },
  });
  return true;
}

bool BlitPass::GenerateMipmaps(std::shared_ptr<Texture> texture,
                                  std::string label) {
  if (!texture) {
    VALIDATION_LOG << "Attempted to add an invalid mipmap generation command "
                      "with no texture.";
    return false;
  }

  commands_.emplace_back(BlitCommand{
      .label = label,
      .data =
          BlitCommand::GenerateMipmaps{
              .texture = texture,
          },
  });
  return true;
}

}  // namespace impeller
