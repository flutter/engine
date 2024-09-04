
#include "flutter/display_list/testing/dl_test_color_source.h"

namespace flutter {
namespace testing {

std::shared_ptr<DlLinearGradientColorSource> MakeLinearColorSource(
    const SkPoint start_point,
    const SkPoint end_point,
    uint32_t stop_count,
    const DlColor* colors,
    const float* stops,
    DlTileMode tile_mode,
    const SkMatrix* matrix) {
  std::vector<DlScalar> components;
  for (uint32_t i = 0; i < stop_count; ++i) {
    DlColor srgb_xr = colors[i].withColorSpace(DlColorSpace::kExtendedSRGB);
    components.push_back(srgb_xr.getAlphaF());
    components.push_back(srgb_xr.getRedF());
    components.push_back(srgb_xr.getGreenF());
    components.push_back(srgb_xr.getBlueF());
  }

  return DlColorSource::MakeLinear(start_point, end_point, stop_count,
                                   components.data(), stops, tile_mode, matrix);
}
}  // namespace testing
}  // namespace flutter
