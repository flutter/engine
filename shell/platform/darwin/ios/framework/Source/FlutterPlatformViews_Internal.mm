#include "flutter/shell/platform/darwin/ios/framework/Source/FlutterPlatformViews_Internal.h"

#include "flutter/shell/platform/darwin/ios/ios_surface.h"

namespace shell {
FlutterPlatformViewLayer::FlutterPlatformViewLayer(fml::scoped_nsobject<UIView> overlay_view,
                         std::unique_ptr<IOSSurface> ios_surface,
                         std::unique_ptr<Surface> surface)
    : overlay_view(std::move(overlay_view)),
      ios_surface(std::move(ios_surface)),
      surface(std::move(surface)){};

FlutterPlatformViewLayer::~FlutterPlatformViewLayer() = default;


FlutterPlatformViewsController::FlutterPlatformViewsController() = default;
FlutterPlatformViewsController::~FlutterPlatformViewsController() = default;


}  // namespace shell
