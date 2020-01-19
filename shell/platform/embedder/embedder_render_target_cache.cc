// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_render_target_cache.h"

namespace flutter {

EmbedderRenderTargetCache::EmbedderRenderTargetCache() = default;

EmbedderRenderTargetCache::~EmbedderRenderTargetCache() = default;

std::pair<EmbedderRenderTargetCache::RenderTargets,
          EmbedderExternalView::ViewIdentifierSet>
EmbedderRenderTargetCache::GetExistingTargetsInCache(
    const EmbedderExternalView::PendingViews& pending_views,
    const IsRenderTargetAvailableCallback& is_render_target_available) {
  RenderTargets resolved_render_targets;
  EmbedderExternalView::ViewIdentifierSet unmatched_identifiers;

  for (const auto& view : pending_views) {
    const auto& external_view = view.second;
    if (!external_view->HasEngineRenderedContents()) {
      continue;
    }

    auto& compatible_targets =
        cached_render_targets_[external_view->CreateRenderTargetDescriptor()];
    while (!compatible_targets.empty()) {
      std::unique_ptr<EmbedderRenderTarget> target =
          std::move(compatible_targets.top());
      compatible_targets.pop();

      if (!is_render_target_available ||
          is_render_target_available(target.get())) {
        resolved_render_targets[view.first] = std::move(target);
        break;
      }
    }

    if (resolved_render_targets.find(view.first) ==
        resolved_render_targets.end()) {
      unmatched_identifiers.insert(view.first);
    }
  }

  return {std::move(resolved_render_targets), std::move(unmatched_identifiers)};
}

std::set<std::unique_ptr<EmbedderRenderTarget>>
EmbedderRenderTargetCache::ClearAllRenderTargetsInCache() {
  std::set<std::unique_ptr<EmbedderRenderTarget>> cleared_targets;
  for (auto& targets : cached_render_targets_) {
    auto& targets_stack = targets.second;
    while (!targets_stack.empty()) {
      cleared_targets.emplace(std::move(targets_stack.top()));
      targets_stack.pop();
    }
  }
  cached_render_targets_.clear();
  return cleared_targets;
}

void EmbedderRenderTargetCache::CacheRenderTarget(
    EmbedderExternalView::ViewIdentifier view_identifier,
    std::unique_ptr<EmbedderRenderTarget> target) {
  if (target == nullptr) {
    return;
  }
  auto surface = target->GetRenderSurface();
  auto desc = EmbedderExternalView::RenderTargetDescriptor{
      view_identifier, SkISize::Make(surface->width(), surface->height())};
  cached_render_targets_[desc].push(std::move(target));
}

size_t EmbedderRenderTargetCache::GetCachedTargetsCount() const {
  size_t count = 0;
  for (const auto& targets : cached_render_targets_) {
    count += targets.second.size();
  }
  return count;
}

}  // namespace flutter
