// Copyright 2022 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_SUITE_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_SUITE_H_

#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/sys/test/cpp/fidl.h>
#include <lib/async/cpp/executor.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/fpromise/promise.h>
#include <lib/sys/cpp/service_directory.h>

#include <map>
#include <memory>
#include <vector>

/// Implement and expose Suite protocol on behalf of wrapped legacy dart test
/// component.
class Suite final : public fuchsia::sys::test::Suite {
  using ComponentMap =
      std::map<run::Component*, std::unique_ptr<run::Component>>;

 public:
  Suite(std::string legacy_url, std::unique_ptr<async::Loop> loop);
  ~Suite() override;

  void GetTests(fidl::InterfaceRequest<fuchsia::sys::test::CaseIterator>
                    iterator) override;

  void Run(
      std::vector<fuchsia::sys::test::Invocation> tests,
      fuchsia::sys::test::RunOptions options,
      fidl::InterfaceHandle<fuchsia::sys::test::RunListener> listener) override;

  fidl::InterfaceRequestHandler<fuchsia::sys::test::Suite> GetHandler() {
    return bindings_.GetHandler(this, dispatcher_);
  }

  void AddBinding(zx::channel request) {
    bindings_.AddBinding(
        this,
        fidl::InterfaceRequest<fuchsia::sys::test::Suite>(std::move(request)),
        dispatcher_);
  }

 private:
  fpromise::promise<> RunTest(
      zx::socket out,
      zx::socket err,
      const std::vector<std::string>& arguments,
      fidl::InterfacePtr<fuchsia::sys::test::CaseListener> case_listener);

  class CaseIterator final : public fuchsia::sys::test::CaseIterator {
   public:
    CaseIterator(
        fidl::InterfaceRequest<fuchsia::sys::test::CaseIterator> request,
        async_dispatcher_t* dispatcher,
        fit::function<void(CaseIterator*)> done_callback);

    void GetNext(GetNextCallback callback) override;

   private:
    int get_next_call_count = 0;
    fidl::Binding<fuchsia::sys::test::CaseIterator> binding_;
    fit::function<void(CaseIterator*)> done_callback_;
  };

  // const std::shared_ptr<sys::ServiceDirectory> test_component_svc_;
  const std::string legacy_url_;
  std::shared_ptr<ComponentMap> test_components_;
  std::map<CaseIterator*, std::unique_ptr<CaseIterator>> case_iterators_;
  async_dispatcher_t* dispatcher_;
  fidl::BindingSet<fuchsia::sys::test::Suite> bindings_;
  async::Executor executor_;

  std::unique_ptr<CaseIterator> RemoveCaseInterator(CaseIterator*);
  std::unique_ptr<run::Component> RemoveComponent(run::Component* ptr);
};

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_SUITE_H_
