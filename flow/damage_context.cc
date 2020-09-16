#include "flutter/flow/damage_context.h"
#include "flutter/flow/layers/layer.h"
#include "third_party/skia/include/core/SkImageFilter.h"

namespace flutter {

void DamageArea::AddRect(const SkRect& rect) {
  SkIRect irect;
  rect.roundOut(&irect);
  bounds_.join(irect);
}

void DamageArea::AddRect(const SkIRect& rect) {
  bounds_.join(rect);
}

std::vector<SkIRect> DamageArea::GetRects() const {
  std::vector<SkIRect> res;
  res.push_back(bounds_);
  return res;
}

std::size_t DamageContext::LayerContribution::Hash::operator()(
    const LayerContribution& e) const noexcept {
  size_t res = e.paint_bounds.left();
  res = 37 * res + e.paint_bounds.top();
  res = 37 * res + e.paint_bounds.width();
  res = 37 * res + e.paint_bounds.height();
  res = 37 * res + reinterpret_cast<std::uintptr_t>(e.comparator);
  return res;
}

namespace {
bool compare_mutators(std::shared_ptr<MutatorNode> m1,
                      std::shared_ptr<MutatorNode> m2) {
  while (true) {
    if (m1 == m2) {
      return true;
    }
    if (!m1 || !m2 || *m1 != *m2) {
      return false;
    }

    m1 = m1->next();
    m2 = m2->next();
  }
}
}  // namespace

bool DamageContext::LayerContribution::operator==(
    const LayerContribution& e) const {
  return comparator == e.comparator && paint_bounds == e.paint_bounds &&
         compare_mutators(mutator_node, e.mutator_node) &&
         (layer.get() == e.layer.get() ||
          comparator(layer.get(), e.layer.get()));
}

void DamageContext::InitFrame(const SkISize& tree_size,
                              const FrameDescription* previous_frame) {
  current_layer_tree_size_ = tree_size;
  previous_frame_ = previous_frame;
}

DamageContext::LayerContributionHandle DamageContext::AddLayerContribution(
    const Layer* layer,
    LayerComparator comparator,
    const SkMatrix& matrix,
    const SkRect& paint_bounds,
    const PrerollContext& preroll_context) {
  LayerContributionHandle res;

  if (current_layer_tree_size_.isEmpty()) {
    return res;
  }

  LayerContribution e;
  e.layer = layer->shared_from_this();
  e.comparator = comparator;

  SkRect bounds = paint_bounds;
  bounds.intersect(preroll_context.cull_rect);
  e.paint_bounds = matrix.mapRect(bounds);
  e.mutator_node = preroll_context.mutators_stack.is_empty()
                       ? nullptr
                       : *preroll_context.mutators_stack.Bottom();

  res.context_ = this;
  res.index_ = layer_entries_.size();
  res.matrix_ = matrix;

  layer_entries_.push_back(std::move(e));

  return res;
}

DamageContext::LayerContributionHandle DamageContext::AddLayerContribution(
    const Layer* layer,
    LayerComparator comparator,
    const SkMatrix& matrix,
    const PrerollContext& preroll_context) {
  return AddLayerContribution(layer, comparator, matrix, layer->paint_bounds(),
                              preroll_context);
}

void DamageContext::AddReadbackArea(const SkMatrix& matrix,
                                    const SkRect& area) {
  SkIRect rect;
  matrix.mapRect(area).roundOut(&rect);
  readback_areas_.push_back(std::move(rect));
}

void DamageContext::LayerContributionHandle::UpdatePaintBounds() {
  if (context_ && index_ != static_cast<size_t>(-1)) {
    LayerContribution& c = context_->layer_entries_[index_];
    SkRect bounds = matrix_.mapRect(c.layer->paint_bounds());
    c.paint_bounds = bounds;
  }
}

DamageContext::DamageResult DamageContext::FinishFrame() {
  DamageResult res;
  res.frame_description.reset(new FrameDescription());
  res.frame_description->layer_tree_size = current_layer_tree_size_;
  auto& entries = res.frame_description->entries;

  for (size_t i = 0; i < layer_entries_.size(); ++i) {
    auto& entry = layer_entries_[i];
    if (entry.paint_bounds.isEmpty()) {
      continue;
    }
    entry.paint_order = i;
    entries.insert(std::move(entry));
  }

  if (!previous_frame_ ||
      previous_frame_->layer_tree_size != current_layer_tree_size_) {
    res.area.AddRect(SkRect::MakeIWH(current_layer_tree_size_.width(),
                                     current_layer_tree_size_.height()));
  } else {
    // layer entries that are only found in one set (only this frame or only
    // previous frame) are for layers that were either added, removed, or
    // modified in any way (fail the equality check in LayerContribution) and
    // thus contribute to damage area

    // matching layer entries from previous frame and this frame; we still need
    // to check for paint order to detect reordered layers
    std::vector<const LayerContribution*> matching_previous;
    std::vector<const LayerContribution*> matching_current;

    for (const auto& l : entries) {
      auto prev = previous_frame_->entries.find(l);
      if (prev == previous_frame_->entries.end()) {
        res.area.AddRect(l.paint_bounds);
      } else {
        matching_current.push_back(&l);
        matching_previous.push_back(&*prev);
      }
    }
    for (const auto& l : previous_frame_->entries) {
      if (entries.find(l) == entries.end()) {
        res.area.AddRect(l.paint_bounds);
      }
    }

    // determine which layers are reordered
    auto comparator = [](const LayerContribution* l1,
                         const LayerContribution* l2) {
      return l1->paint_order < l2->paint_order;
    };
    std::sort(matching_previous.begin(), matching_previous.end(), comparator);
    std::sort(matching_current.begin(), matching_current.end(), comparator);

    // We have two sets of matching layer entries that possibly differ in paint
    // order and we sorted them by paint order, i.e.
    // B C D E
    // ^-- prev
    // C D B E
    // ^-- cur
    // 1. move cur until match with prev is found (B)
    // all layers before cur that intersect with prev are reordered and will
    // contribute to damage, as does prev
    // 2. remove prev and cur (now both pointing at B)
    // 3. repeat until empty
    // (except we actually do it in reverse to not erase from beginning)
    while (!matching_previous.empty()) {
      auto prev = matching_previous.end() - 1;
      auto cur = matching_current.end() - 1;

      while (*(*prev) != *(*cur)) {
        if ((*prev)->paint_bounds.intersects((*cur)->paint_bounds)) {
          res.area.AddRect((*prev)->paint_bounds);
          res.area.AddRect((*cur)->paint_bounds);
        }
        --cur;
      }
      matching_previous.erase(prev);
      matching_current.erase(cur);
    }
  }

  // If anything is damaged a in readback area, the whole readback area needs
  // to be repainted
  for (const auto& readback : readback_areas_) {
    if (SkIRect::Intersects(res.area.bounds(), readback)) {
      res.area.AddRect(readback);
    }
  }

  previous_frame_ = nullptr;
  layer_entries_.clear();
  readback_areas_.clear();
  current_layer_tree_size_ = SkISize::MakeEmpty();

  return res;
}

}  // namespace flutter
