// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/sky/content_handler_impl.h"

#include "base/bind.h"
#include "mojo/common/binding_set.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/cpp/application/service_provider_impl.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/public/cpp/utility/run_loop.h"
#include "mojo/services/network/interfaces/network_service.mojom.h"
#include "mojo/services/ui/views/interfaces/view_provider.mojom.h"
#include "services/sky/document_view.h"

namespace sky {

class SkyViewProvider : public mojo::InterfaceFactory<mojo::ui::ViewProvider>,
                        public mojo::ui::ViewProvider {
 public:
  SkyViewProvider(mojo::InterfaceRequest<mojo::ServiceProvider> services,
                  mojo::URLResponsePtr response,
                  mojo::Shell* shell)
      : service_provider_(services.Pass()),
        response_(response.Pass()),
        shell_(shell) {
    service_provider_.AddService(this);
  }

  ~SkyViewProvider() override {}

  // InterfaceFactory:
  void Create(mojo::ApplicationConnection* connection,
              mojo::InterfaceRequest<mojo::ui::ViewProvider> request) override {
    bindings_.AddBinding(this, request.Pass());
  }

  // ViewProvider:
  void CreateView(mojo::InterfaceRequest<mojo::ServiceProvider> services,
                  mojo::ServiceProviderPtr exposed_services,
                  const CreateViewCallback& callback) override {
    // TODO(jeffbrown): We shouldn't crash if a second request to create
    // a view comes in.  Ideally we should just be able to make a second
    // instance of the view using the same content we just downloaded from
    // the network.  Fix this later.
    DCHECK(response_.get());
    new DocumentView(services.Pass(), exposed_services.Pass(), response_.Pass(),
                     shell_, callback);
  }

 private:
  mojo::ServiceProviderImpl service_provider_;
  mojo::URLResponsePtr response_;
  mojo::Shell* shell_;
  mojo::BindingSet<mojo::ui::ViewProvider> bindings_;
};

class SkyApplication : public mojo::Application {
 public:
  SkyApplication(mojo::InterfaceRequest<mojo::Application> application,
                 mojo::URLResponsePtr response)
      : binding_(this, application.Pass()),
        initial_response_(response.Pass()) {}

  void Initialize(mojo::ShellPtr shell,
                  mojo::Array<mojo::String> args,
                  const mojo::String& url) override {
    shell_ = shell.Pass();
    mojo::ServiceProviderPtr service_provider;
    shell_->ConnectToApplication("mojo:authenticated_network_service",
                                 mojo::GetProxy(&service_provider), nullptr);
    mojo::ConnectToService(service_provider.get(), &network_service_);
  }

  void AcceptConnection(const mojo::String& requestor_url,
                        mojo::InterfaceRequest<mojo::ServiceProvider> services,
                        mojo::ServiceProviderPtr exposed_services,
                        const mojo::String& url) override {
    if (initial_response_) {
      OnResponseReceived(mojo::URLLoaderPtr(), services.Pass(),
                         exposed_services.Pass(), initial_response_.Pass());
    } else {
      mojo::URLLoaderPtr loader;
      network_service_->CreateURLLoader(mojo::GetProxy(&loader));
      mojo::URLRequestPtr request(mojo::URLRequest::New());
      request->url = url;
      request->auto_follow_redirects = true;

      // |loader| will be pass to the OnResponseReceived method through a
      // callback. Because order of evaluation is undefined, a reference to the
      // raw pointer is needed.
      mojo::URLLoader* raw_loader = loader.get();
      raw_loader->Start(
          request.Pass(),
          base::Bind(&SkyApplication::OnResponseReceived,
                     base::Unretained(this), base::Passed(&loader),
                     base::Passed(&services), base::Passed(&exposed_services)));
    }
  }

  void RequestQuit() override { mojo::RunLoop::current()->Quit(); }

 private:
  void OnResponseReceived(
      mojo::URLLoaderPtr loader,
      mojo::InterfaceRequest<mojo::ServiceProvider> services,
      mojo::ServiceProviderPtr exposed_services,
      mojo::URLResponsePtr response) {
    new SkyViewProvider(services.Pass(), response.Pass(), shell_.get());
  }

  mojo::StrongBinding<mojo::Application> binding_;
  mojo::ShellPtr shell_;
  mojo::NetworkServicePtr network_service_;
  mojo::URLResponsePtr initial_response_;
};

ContentHandlerImpl::ContentHandlerImpl(
    mojo::InterfaceRequest<mojo::ContentHandler> request)
    : binding_(this, request.Pass()) {}

ContentHandlerImpl::~ContentHandlerImpl() {}

void ContentHandlerImpl::StartApplication(
    mojo::InterfaceRequest<mojo::Application> application,
    mojo::URLResponsePtr response) {
  new SkyApplication(application.Pass(), response.Pass());
}

}  // namespace sky
