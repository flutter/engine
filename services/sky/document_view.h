// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_SKY_DOCUMENT_VIEW_H_
#define SERVICES_SKY_DOCUMENT_VIEW_H_

#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/application/lazy_interface_ptr.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/public/interfaces/application/application.mojom.h"
#include "mojo/services/asset_bundle/interfaces/asset_bundle.mojom.h"
#include "mojo/services/content_handler/interfaces/content_handler.mojom.h"
#include "mojo/services/input_events/interfaces/input_events.mojom.h"
#include "mojo/services/navigation/interfaces/navigation.mojom.h"
#include "mojo/services/network/interfaces/url_loader.mojom.h"
#include "mojo/services/service_registry/interfaces/service_registry.mojom.h"
#include "mojo/services/ui/views/interfaces/views.mojom.h"
#include "mojo/services/ui/views/interfaces/view_manager.mojom.h"
#include "mojo/services/ui/views/interfaces/view_provider.mojom.h"
#include "services/sky/compositor/layer_client.h"
#include "services/sky/compositor/layer_host_client.h"
#include "sky/compositor/layer_tree.h"
#include "sky/engine/public/platform/ServiceProvider.h"
#include "sky/engine/public/sky/sky_view.h"
#include "sky/engine/public/sky/sky_view_client.h"

namespace sky {
class DartLibraryProviderImpl;
class LayerHost;
class Rasterizer;
class RasterizerBitmap;
class TextureLayer;

class DocumentView : public blink::ServiceProvider,
                     public blink::SkyViewClient,
                     public mojo::ui::View,
                     public sky::LayerClient,
                     public sky::LayerHostClient {
 public:
  DocumentView(mojo::InterfaceRequest<mojo::ServiceProvider> exported_services,
               mojo::ServiceProviderPtr imported_services,
               mojo::URLResponsePtr response,
               mojo::Shell* shell,
               const mojo::ui::ViewProvider::CreateViewCallback& callback);
  ~DocumentView() override;

  base::WeakPtr<DocumentView> GetWeakPtr();

  mojo::Shell* shell() const { return shell_; }

  // sky::LayerHostClient
  mojo::Shell* GetShell() override;
  void BeginFrame(base::TimeTicks frame_time) override;
  void OnSurfaceIdAvailable(mojo::SurfaceIdPtr surface_id) override;
  // sky::LayerClient
  void PaintContents(SkCanvas* canvas, const gfx::Rect& clip) override;

  // SkyViewClient methods:
  void ScheduleFrame() override;

  void StartDebuggerInspectorBackend();

  void GetPixelsForTesting(std::vector<unsigned char>* pixels);

  mojo::ScopedMessagePipeHandle TakeRootBundleHandle();
  mojo::ScopedMessagePipeHandle TakeServicesProvidedToEmbedder();
  mojo::ScopedMessagePipeHandle TakeServicesProvidedByEmbedder();
  mojo::ScopedMessagePipeHandle TakeServiceRegistry();

 private:
  // SkyViewClient methods:
  void DidCreateIsolate(Dart_Isolate isolate) override;

  // Services methods:
  mojo::NavigatorHost* NavigatorHost() override;

  // View methods:
  void OnLayout(mojo::ui::ViewLayoutParamsPtr layout_params,
                mojo::Array<uint32_t> children_needing_layout,
                const OnLayoutCallback& callback) override;
  void OnChildUnavailable(uint32_t child_key,
                          const OnChildUnavailableCallback& callback) override;

  void OnViewConnectionError();
  void FinishLayout();

  void Load(mojo::URLResponsePtr response);
  float GetDevicePixelRatio() const;
  scoped_ptr<Rasterizer> CreateRasterizer();

  void LoadFromSnapshotStream(String name,
                              mojo::ScopedDataPipeConsumerHandle snapshot);

  void HandleInputEvent(mojo::EventPtr event);

  void InitServiceRegistry();

  mojo::URLResponsePtr response_;
  mojo::InterfaceRequest<mojo::ServiceProvider> exported_services_;
  mojo::ServiceProviderPtr imported_services_;
  mojo::Shell* shell_;
  mojo::asset_bundle::AssetBundlePtr root_bundle_;
  mojo::NavigatorHostPtr navigator_host_;
  std::unique_ptr<blink::SkyView> sky_view_;
  scoped_ptr<DartLibraryProviderImpl> library_provider_;
  scoped_ptr<LayerHost> layer_host_;
  scoped_refptr<TextureLayer> root_layer_;
  std::unique_ptr<compositor::LayerTree>
      current_layer_tree_;  // TODO(abarth): Integrate //sky/compositor and
                            // //services/sky/compositor.
  compositor::PaintContext paint_context_;
  RasterizerBitmap* bitmap_rasterizer_;  // Used for pixel tests.
  mojo::ServiceRegistryPtr service_registry_;

  mojo::Binding<mojo::ui::View> view_binding_;
  mojo::ui::ViewManagerPtr view_manager_;
  mojo::ui::ViewHostPtr view_host_;
  OnLayoutCallback pending_layout_callback_;
  mojo::Size size_;
  float device_pixel_ratio_;
  mojo::SurfaceIdPtr surface_id_;

  base::WeakPtrFactory<DocumentView> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(DocumentView);
};

}  // namespace sky

#endif  // SERVICES_SKY_DOCUMENT_VIEW_H_
