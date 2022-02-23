// Copyright 2022 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "suite.h"

#include <fuchsia/component/runner/cpp/fidl.h>
#include <fuchsia/io/cpp/fidl.h>
#include <fuchsia/logger/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/sys/test/cpp/fidl.h>
#include <lib/async/cpp/executor.h>
#include <lib/async/cpp/task.h>
#include <lib/async/dispatcher.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_ptr.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/fpromise/barrier.h>
#include <lib/fpromise/bridge.h>
#include <lib/fpromise/promise.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/sys/cpp/termination_reason.h>
#include <lib/sys/cpp/testing/enclosing_environment.h>
// #include <lib/syslog/cpp/macros.h>
#include <lib/zx/socket.h>
#include <unistd.h>
#include <zircon/processargs.h>
#include <zircon/status.h>

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "flutter/fml/logging.h"
#include "string_printf.h"

static const char kTestCaseName[] = "dart_test";
// constexpr char kEnvPrefix[] = "test_env_";

Suite::Suite(std::string legacy_url, async_dispatcher_t* dispatcher)
    : legacy_url_(std::move(legacy_url)),
      test_components_(std::make_shared<ComponentMap>()),
      dispatcher_(dispatcher),
      executor_(dispatcher) {
  FML_LOG(INFO) << "SUITE CLASS CREATED";
}

Suite::~Suite() = default;

Suite::CaseIterator::CaseIterator(
    fidl::InterfaceRequest<fuchsia::sys::test::CaseIterator> request,
    async_dispatcher_t* dispatcher,
    fit::function<void(CaseIterator*)> done_callback)
    : binding_(this, std::move(request), dispatcher),
      done_callback_(std::move(done_callback)) {}

void Suite::CaseIterator::GetNext(GetNextCallback callback) {
  if (get_next_call_count == 0) {
    fuchsia::sys::test::Case test_case;
    test_case.set_name(std::string(kTestCaseName));
    test_case.set_enabled(true);
    std::vector<fuchsia::sys::test::Case> cases;
    cases.push_back(std::move(test_case));
    callback(std::move(cases));
    get_next_call_count++;

  } else {
    std::vector<fuchsia::sys::test::Case> cases;
    callback(std::move(cases));
    // this would be removed
    done_callback_(this);
  }
}

void Suite::GetTests(
    fidl::InterfaceRequest<fuchsia::sys::test::CaseIterator> iterator) {
  FML_LOG(INFO) << "DART RUNNER HIT SUITE GETTESTS";
  auto case_iterator = std::make_unique<CaseIterator>(
      std::move(iterator), dispatcher_, [this](CaseIterator* case_iterator) {
        RemoveCaseInterator(case_iterator);
      });
  case_iterators_.emplace(case_iterator.get(), std::move(case_iterator));
}

std::unique_ptr<Suite::CaseIterator> Suite::RemoveCaseInterator(
    CaseIterator* case_iterator) {
  auto it = case_iterators_.find(case_iterator);
  std::unique_ptr<Suite::CaseIterator> case_iterator_ptr;
  if (it != case_iterators_.end()) {
    case_iterator_ptr = std::move(it->second);
    case_iterators_.erase(it);
  }
  return case_iterator_ptr;
}

void Suite::Run(
    std::vector<fuchsia::sys::test::Invocation> tests,
    fuchsia::sys::test::RunOptions options,
    fidl::InterfaceHandle<fuchsia::sys::test::RunListener> listener) {
  FML_LOG(INFO) << "DART RUNNER HIT SUITE RUN";
  auto listener_proxy = listener.Bind();
  std::vector<std::string> args;
  if (options.has_arguments()) {
    args = std::move(*options.mutable_arguments());
  }

  if (options.has_parallel()) {
    FML_LOG(WARNING) << "Ignoring 'parallel'. Pass test specific flags, eg: "
                        "for rust test pass in "
                        "--test-threads="
                     << options.parallel();
  }

  fpromise::barrier barrier;
  for (auto it = tests.begin(); it != tests.end(); it++) {
    auto invocation = std::move(*it);
    std::string test_case_name;
    if (invocation.has_name()) {
      test_case_name = invocation.name();
    }
    FML_LOG(INFO) << "DART RUNNER HIT SUITE RUN - TEST CASE: "
                  << test_case_name;
    zx::socket out, err, out_client, err_client;
    auto status = zx::socket::create(0, &out, &out_client);
    if (status != ZX_OK) {
      FML_LOG(FATAL) << "cannot create socket: "
                     << zx_status_get_string(status);
    }
    status = zx::socket::create(0, &err, &err_client);
    if (status != ZX_OK) {
      FML_LOG(FATAL) << "cannot create socket: "
                     << zx_status_get_string(status);
    }

    fidl::InterfacePtr<fuchsia::sys::test::CaseListener> case_listener;

    fuchsia::sys::test::StdHandles std_handles;
    std_handles.set_err(std::move(err_client));
    std_handles.set_out(std::move(out_client));

    listener_proxy->OnTestCaseStarted(std::move(invocation),
                                      std::move(std_handles),
                                      case_listener.NewRequest());
    // if (test_case_name != kTestCaseName) {
    //   FML_LOG(INFO) << "TEST CASE: " << test_case_name;
    //   FML_LOG(INFO) << "kTEST CASE: " << kTestCaseName;
    //   std::string msg =
    //       fxl::StringPrintf("Invalid test case, expected: %s, got: %s\n",
    //                         kTestCaseName, test_case_name.c_str());
    //   err.write(0, msg.c_str(), msg.length(), nullptr);
    //   fuchsia::sys::test::Result result;
    //   result.set_status(fuchsia::sys::test::Status::FAILED);
    //   case_listener->Finished(std::move(result));
    // } else {
    //   auto promise = RunTest(std::move(out), std::move(err), args,
    //                          std::move(case_listener))
    //                      .wrap_with(barrier);
    //   executor_.schedule_task(std::move(promise));
    // }
  }

  auto sync_promise =
      fpromise::make_promise([listener_proxy = std::move(listener_proxy)] {
        FML_LOG(INFO) << "Sending OnFinished for legacy tests";
        listener_proxy->OnFinished();
      });
  executor_.schedule_task(barrier.sync().and_then(std::move(sync_promise)));
}

// fpromise::promise<> Suite::RunTest(
//     zx::socket out,
//     zx::socket err,
//     const std::vector<std::string>& arguments,
//     fidl::InterfacePtr<fuchsia::sys::test::CaseListener> case_listener) {
//   sys::testing::EnvironmentServices::ParentOverrides parent_overrides;
//   parent_overrides.debug_data_service_ = std::make_shared<vfs::Service>(
//       [namespace_services = test_component_svc_](
//           zx::channel channel, async_dispatcher_t* /*unused*/) {
//         // namespace_services->Connect(
//         //     fuchsia::sys::test::DebugData::Name_,
//         //     std::move(channel));
//       });

//   auto test_env_services =
//       sys::testing::EnvironmentServices::CreateWithParentOverrides(
//           parent_env_, std::move(parent_overrides));

//   // Add these services to the environment if they are not injected and test
//   // doesn't request sys version of them.
//   std::set<std::string> services_to_add = {
//       fuchsia::logger::LogSink::Name_, fuchsia::logger::Log::Name_,
//       fuchsia::diagnostics::ArchiveAccessor::Name_};

//   // Compute a common random suffix for the environments created to run the
//   // test.
//   uint32_t env_rand_suffix;
//   zx_cprng_draw(&env_rand_suffix, sizeof(env_rand_suffix));

//   for (const auto& service : services_to_add) {
//     test_env_services->AddService<void>(
//         [namespace_services = test_component_svc_,
//          service](fidl::InterfaceRequest<void> request) {
//           namespace_services->Connect(service, request.TakeChannel());
//         },
//         service);
//   }

//   fuchsia::sys::EnvironmentOptions env_opt;
//   // std::string env_label = std::format("%s%08x", kEnvPrefix,
//   // env_rand_suffix);
//   std::string env_label = kEnvPrefix;
//   env_opt.delete_storage_on_death = true;

//   auto enclosing_env = sys::testing::EnclosingEnvironment::Create(
//       env_label, parent_env_, std::move(test_env_services), env_opt);

//   auto launcher = enclosing_env->launcher_ptr();

//   auto info =
//       fuchsia::sys::LaunchInfo{.url = legacy_url_, .arguments = arguments};
//   auto out_collector = AddOutputFileDescriptor(STDOUT_FILENO, std::move(out),
//                                                dispatcher_, &info.out);
//   auto err_collector = AddOutputFileDescriptor(STDERR_FILENO, std::move(err),
//                                                dispatcher_, &info.err);

//   auto svc =
//   sys::ServiceDirectory::CreateWithRequest(&info.directory_request);

//   // fuchsia::component::runner::ComponentControllerPtr contoller;
//   fuchsia::sys::ComponentControllerPtr contoller;
//   launcher->CreateComponent(std::move(info), contoller.NewRequest());
//   auto test_component = std::make_unique<run::Component>(
//       std::move(out_collector), std::move(err_collector),
//       std::move(contoller), std::move(svc));
//   fpromise::bridge<> bridge;
//   test_component->controller().events().OnTerminated =
//       [this, url = legacy_url_, enclosing_env = std::move(enclosing_env),
//        completer = std::move(bridge.completer),
//        case_listener = std::move(case_listener),
//        this_ptr = test_component.get()](
//           int64_t return_code,
//           fuchsia::sys::TerminationReason termination_reason) mutable {
//         if (termination_reason != fuchsia::sys::TerminationReason::EXITED) {
//           FML_LOG(WARNING) << "Test " << url << " failed with "
//                            << sys::HumanReadableTerminationReason(
//                                   termination_reason);
//         }

//         FML_LOG(INFO) << "Legacy test exited with return code " <<
//         return_code
//                       << ", collecting stdout";

//         auto status = return_code == 0 ? fuchsia::sys::test::Status::PASSED
//                                        : fuchsia::sys::test::Status::FAILED;

//         auto output_promise = this_ptr->SignalWhenOutputCollected();

//         FML_LOG(INFO) << "Killing environment for legacy test";
//         fpromise::bridge<> bridge;
//         enclosing_env->Kill(bridge.completer.bind());
//         auto promise = bridge.consumer.promise().and_then(
//             [enclosing_env = std::move(enclosing_env),
//              case_listener = std::move(case_listener), status = status,
//              completer = std::move(completer)]() mutable {
//               fuchsia::sys::test::Result result;
//               result.set_status(status);
//               FML_LOG(INFO) << "Sending finished event for legacy tests";
//               case_listener->Finished(std::move(result));
//               completer.complete_ok();
//             });
//         this->executor_.schedule_task(output_promise.and_then([this,
//                                                                this_ptr]() {
//           FML_LOG(INFO) << "Done collecting standard output for legacy tests
//           "; RemoveComponent(this_ptr);
//         }));
//         this->executor_.schedule_task(std::move(promise));
//       };

//   test_components_->emplace(test_component.get(), std::move(test_component));

//   return bridge.consumer.promise();
// }

// std::unique_ptr<run::Component> Suite::RemoveComponent(run::Component* ptr) {
//   if (!test_components_) {
//     return nullptr;
//   }
//   auto it = test_components_->find(ptr);
//   std::unique_ptr<run::Component> test_component_ptr;
//   if (it != test_components_->end()) {
//     test_component_ptr = std::move(it->second);
//     test_components_->erase(it);
//   }
//   return test_component_ptr;
// }
