// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/entity_pass_clip_stack.h"
#include "impeller/entity/contents/clip_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"

namespace impeller {

EntityPassClipStack::EntityPassClipStack() {}

std::optional<Rect> EntityPassClipStack::CurrentClipCoverage() const {
  return clip_coverage_.back().back().coverage;
}

bool EntityPassClipStack::HasCoverage() const {
  return !clip_coverage_.back().empty();
}

void EntityPassClipStack::Initialize(const Rect& rect) {
  clip_coverage_.clear();
  clip_coverage_.push_back({ClipCoverageLayer{
      .coverage = rect,
      .clip_depth = 0,
  }});
}

void EntityPassClipStack::PushSubpass(std::optional<Rect> subpass_coverage,
                                      size_t clip_depth) {
  clip_coverage_.push_back({
      ClipCoverageLayer{.coverage = subpass_coverage, .clip_depth = clip_depth},
  });
}

void EntityPassClipStack::PopSubpass() {
  clip_coverage_.pop_back();
}

const std::vector<std::vector<ClipCoverageLayer>>
EntityPassClipStack::GetClipCoverageLayers() const {
  return clip_coverage_;
}

bool EntityPassClipStack::AppendClipCoverage(
    Contents::ClipCoverage clip_coverage,
    Entity& entity,
    size_t clip_depth_floor,
    Point global_pass_position) {
  switch (clip_coverage.type) {
    case Contents::ClipCoverage::Type::kNoChange:
      break;
    case Contents::ClipCoverage::Type::kAppend: {
      auto op = CurrentClipCoverage();
      clip_coverage_.back().push_back(
          ClipCoverageLayer{.coverage = clip_coverage.coverage,
                            .clip_depth = entity.GetClipDepth() + 1});

      FML_DCHECK(clip_coverage_.back().back().clip_depth ==
                 clip_coverage_.back().front().clip_depth +
                     clip_coverage_.back().size() - 1);

      if (!op.has_value()) {
        // Running this append op won't impact the clip buffer because the
        // whole screen is already being clipped, so skip it.
        return false;
      }
    } break;
    case Contents::ClipCoverage::Type::kRestore: {
      if (clip_coverage_.back().back().clip_depth <= entity.GetClipDepth()) {
        // Drop clip restores that will do nothing.
        return false;
      }

      auto restoration_index =
          entity.GetClipDepth() - clip_coverage_.back().front().clip_depth;
      FML_DCHECK(restoration_index < clip_coverage_.back().size());

      // We only need to restore the area that covers the coverage of the
      // clip rect at target depth + 1.
      std::optional<Rect> restore_coverage =
          (restoration_index + 1 < clip_coverage_.back().size())
              ? clip_coverage_.back()[restoration_index + 1].coverage
              : std::nullopt;
      if (restore_coverage.has_value()) {
        // Make the coverage rectangle relative to the current pass.
        restore_coverage = restore_coverage->Shift(-global_pass_position);
      }
      clip_coverage_.back().resize(restoration_index + 1);

      if constexpr (ContentContext::kEnableStencilThenCover) {
        // Skip all clip restores when stencil-then-cover is enabled.
        if (clip_coverage_.back().back().coverage.has_value()) {
          RecordEntity(entity, clip_coverage.type);
        }
        return false;
      }

      if (!clip_coverage_.back().back().coverage.has_value()) {
        // Running this restore op won't make anything renderable, so skip it.
        return false;
      }

      auto restore_contents =
          static_cast<ClipRestoreContents*>(entity.GetContents().get());
      restore_contents->SetRestoreCoverage(restore_coverage);

    } break;
  }

#ifdef IMPELLER_ENABLE_CAPTURE
  {
    auto element_entity_coverage = entity.GetCoverage();
    if (element_entity_coverage.has_value()) {
      element_entity_coverage =
          element_entity_coverage->Shift(global_pass_position);
      entity.GetCapture().AddRect("Coverage", *element_entity_coverage,
                                  {.readonly = true});
    }
  }
#endif

  entity.SetClipDepth(entity.GetClipDepth() - clip_depth_floor);
  RecordEntity(entity, clip_coverage.type);

  return true;
}

void EntityPassClipStack::RecordEntity(const Entity& entity,
                                       Contents::ClipCoverage::Type type) {
  switch (type) {
    case Contents::ClipCoverage::Type::kNoChange:
      return;
    case Contents::ClipCoverage::Type::kAppend:
      rendered_clip_entities_.push_back(entity.Clone());
      break;
    case Contents::ClipCoverage::Type::kRestore:
      if (!rendered_clip_entities_.empty()) {
        rendered_clip_entities_.pop_back();
      }
      break;
  }
}

const std::vector<Entity>& EntityPassClipStack::GetReplayEntities() const {
  return rendered_clip_entities_;
}

}  // namespace impeller
