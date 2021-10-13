// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/shell.h"

#include <chrono>
#include <thread>

#include "flutter/benchmarking/benchmarking.h"
#include "flutter/fml/logging.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/shell/gpu/gpu_surface_software.h"
#include "flutter/testing/elf_loader.h"
#include "flutter/testing/testing.h"

namespace flutter {
class BenchmarkExternalViewEmbedder : public ExternalViewEmbedder {
  // |ExternalViewEmbedder|
  SkCanvas* GetRootCanvas() override { return nullptr; }

  // |ExternalViewEmbedder|
  void CancelFrame() override {}

  // |ExternalViewEmbedder|
  void BeginFrame(
      SkISize frame_size,
      GrDirectContext* context,
      double device_pixel_ratio,
      fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) override {}

  // |ExternalViewEmbedder|
  void PrerollCompositeEmbeddedView(
      int view_id,
      std::unique_ptr<EmbeddedViewParams> params) override {}

  // |ExternalViewEmbedder|
  std::vector<SkCanvas*> GetCurrentCanvases() override { return {&canvas_}; }

  // |ExternalViewEmbedder|
  SkCanvas* CompositeEmbeddedView(int view_id) override { return &canvas_; }

 private:
  SkCanvas canvas_;
};

class BenchmarkPlatformView : public PlatformView,
                              public GPUSurfaceSoftwareDelegate {
 public:
  BenchmarkPlatformView(Delegate& delegate, TaskRunners task_runners)
      : PlatformView(delegate, std::move(task_runners)) {}

  // |PlatformView|
  std::unique_ptr<Surface> CreateRenderingSurface() override {
    auto surface = std::make_unique<GPUSurfaceSoftware>(
        this, true /* render to surface */);
    FML_DCHECK(surface->IsValid());
    return surface;
  }

  // |GPUSurfaceSoftwareDelegate|
  sk_sp<SkSurface> AcquireBackingStore(const SkISize& size) override {
    if (sk_surface_ != nullptr &&
        SkISize::Make(sk_surface_->width(), sk_surface_->height()) == size) {
      // The old and new surface sizes are the same. Nothing to do here.
      return sk_surface_;
    }

    SkImageInfo info =
        SkImageInfo::MakeN32(size.fWidth, size.fHeight, kPremul_SkAlphaType,
                             SkColorSpace::MakeSRGB());
    sk_surface_ = SkSurface::MakeRaster(info, nullptr);

    if (sk_surface_ == nullptr) {
      FML_LOG(ERROR)
          << "Could not create backing store for software rendering.";
      return nullptr;
    }

    return sk_surface_;
  }

  // |GPUSurfaceSoftwareDelegate|
  bool PresentBackingStore(sk_sp<SkSurface> backing_store) override {
    return true;
  }

  // |PlatformView|
  std::shared_ptr<ExternalViewEmbedder> CreateExternalViewEmbedder() override {
    return external_view_embedder_;
  }

 private:
  sk_sp<SkSurface> sk_surface_ = nullptr;
  std::shared_ptr<BenchmarkExternalViewEmbedder> external_view_embedder_ =
      std::make_shared<BenchmarkExternalViewEmbedder>();
};

static void StartupAndShutdownShell(benchmark::State& state,
                                    bool measure_startup,
                                    bool measure_shutdown) {
  auto assets_dir = fml::OpenDirectory(testing::GetFixturesPath(), false,
                                       fml::FilePermission::kRead);
  std::unique_ptr<Shell> shell;
  std::unique_ptr<ThreadHost> thread_host;
  testing::ELFAOTSymbols aot_symbols;

  {
    benchmarking::ScopedPauseTiming pause(state, !measure_startup);
    Settings settings = {};
    settings.task_observer_add = [](intptr_t, fml::closure) {};
    settings.task_observer_remove = [](intptr_t) {};

    if (DartVM::IsRunningPrecompiledCode()) {
      aot_symbols = testing::LoadELFSymbolFromFixturesIfNeccessary(
          testing::kDefaultAOTAppELFFileName);
      FML_CHECK(
          testing::PrepareSettingsForAOTWithSymbols(settings, aot_symbols))
          << "Could not set up settings with AOT symbols.";
    } else {
      settings.application_kernels = [&]() {
        std::vector<std::unique_ptr<const fml::Mapping>> kernel_mappings;
        kernel_mappings.emplace_back(
            fml::FileMapping::CreateReadOnly(assets_dir, "kernel_blob.bin"));
        return kernel_mappings;
      };
    }

    thread_host = std::make_unique<ThreadHost>(
        "io.flutter.bench.", ThreadHost::Type::Platform |
                                 ThreadHost::Type::RASTER |
                                 ThreadHost::Type::IO | ThreadHost::Type::UI);

    TaskRunners task_runners("test",
                             thread_host->platform_thread->GetTaskRunner(),
                             thread_host->raster_thread->GetTaskRunner(),
                             thread_host->ui_thread->GetTaskRunner(),
                             thread_host->io_thread->GetTaskRunner());

    shell = Shell::Create(
        flutter::PlatformData(), std::move(task_runners), settings,
        [](Shell& shell) {
          return std::make_unique<PlatformView>(shell, shell.GetTaskRunners());
        },
        [](Shell& shell) { return std::make_unique<Rasterizer>(shell); });
  }

  FML_CHECK(shell);

  {
    // The ui thread could be busy processing tasks after shell created, e.g.,
    // default font manager setup. The measurement of shell shutdown should be
    // considered after those ui tasks have been done.
    //
    // However, if we're measuring the complete time from startup to shutdown,
    // this time should still be included.
    benchmarking::ScopedPauseTiming pause(
        state, !measure_shutdown || !measure_startup);
    fml::AutoResetWaitableEvent latch;
    fml::TaskRunner::RunNowOrPostTask(thread_host->ui_thread->GetTaskRunner(),
                                      [&latch]() { latch.Signal(); });
    latch.Wait();
  }

  {
    benchmarking::ScopedPauseTiming pause(state, !measure_shutdown);
    // Shutdown must occur synchronously on the platform thread.
    fml::AutoResetWaitableEvent latch;
    fml::TaskRunner::RunNowOrPostTask(
        thread_host->platform_thread->GetTaskRunner(),
        [&shell, &latch]() mutable {
          shell.reset();
          latch.Signal();
        });
    latch.Wait();
    thread_host.reset();
  }

  FML_CHECK(!shell);
}

static void BM_ShellInitialization(benchmark::State& state) {
  while (state.KeepRunning()) {
    StartupAndShutdownShell(state, true, false);
  }
}

BENCHMARK(BM_ShellInitialization);

static void BM_ShellShutdown(benchmark::State& state) {
  while (state.KeepRunning()) {
    StartupAndShutdownShell(state, false, true);
  }
}

BENCHMARK(BM_ShellShutdown);

static void BM_ShellInitializationAndShutdown(benchmark::State& state) {
  while (state.KeepRunning()) {
    StartupAndShutdownShell(state, true, true);
  }
}

BENCHMARK(BM_ShellInitializationAndShutdown);

static void BM_ShellOnPlatformViewCreated(benchmark::State& state) {
  std::unique_ptr<Shell> shell;
  std::unique_ptr<ThreadHost> thread_host;

  auto assets_dir = fml::OpenDirectory(testing::GetFixturesPath(), false,
                                       fml::FilePermission::kRead);
  testing::ELFAOTSymbols aot_symbols;

  Settings settings = {};
  settings.task_observer_add = [](intptr_t, fml::closure) {};
  settings.task_observer_remove = [](intptr_t) {};

  if (DartVM::IsRunningPrecompiledCode()) {
    aot_symbols = testing::LoadELFSymbolFromFixturesIfNeccessary(
        testing::kDefaultAOTAppELFFileName);
    FML_CHECK(testing::PrepareSettingsForAOTWithSymbols(settings, aot_symbols))
        << "Could not set up settings with AOT symbols.";
  } else {
    settings.application_kernels = [&]() {
      std::vector<std::unique_ptr<const fml::Mapping>> kernel_mappings;
      kernel_mappings.emplace_back(
          fml::FileMapping::CreateReadOnly(assets_dir, "kernel_blob.bin"));
      return kernel_mappings;
    };
  }

  thread_host = std::make_unique<ThreadHost>(
      "io.flutter.bench.", ThreadHost::Type::Platform |
                               ThreadHost::Type::RASTER | ThreadHost::Type::IO |
                               ThreadHost::Type::UI);

  TaskRunners task_runners("test",
                           thread_host->platform_thread->GetTaskRunner(),
                           thread_host->raster_thread->GetTaskRunner(),
                           thread_host->ui_thread->GetTaskRunner(),
                           thread_host->io_thread->GetTaskRunner());

  shell = Shell::Create(
      flutter::PlatformData(), std::move(task_runners), settings,
      [](Shell& shell) {
        return std::make_unique<BenchmarkPlatformView>(shell,
                                                       shell.GetTaskRunners());
      },
      [](Shell& shell) { return std::make_unique<Rasterizer>(shell); });

  FML_CHECK(shell);

  while (state.KeepRunning()) {
    // Artificially busy the UI thread, as if loading a larger program
    // snapshot.
    fml::TaskRunner::RunNowOrPostTask(
        thread_host->ui_thread->GetTaskRunner(),
        []() { std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    shell->GetPlatformView()->NotifyCreated();
    {
      benchmarking::ScopedPauseTiming pause(state);
      shell->GetPlatformView()->NotifyDestroyed();
    }
  }

  // Shutdown must occur synchronously on the platform thread.
  fml::AutoResetWaitableEvent latch;
  fml::TaskRunner::RunNowOrPostTask(
      thread_host->platform_thread->GetTaskRunner(),
      [&shell, &latch]() mutable {
        shell.reset();
        latch.Signal();
      });
  latch.Wait();
  thread_host.reset();
}

BENCHMARK(BM_ShellOnPlatformViewCreated);

}  // namespace flutter
