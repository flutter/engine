// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// It is __imperative__ that the functions in this file are __not__ included in
// release or profile builds.
//
// They call into the "private" ptrace() API to ensure that the current process
// is being ptrace()-d. Only debug builds rely on ptrace(), and the ptrace() API
// is not allowed for use in the App Store, so we must exclude it from profile-
// and release-builds.
//
// When an app is launched from a host workstation (e.g. via Xcode or
// "ios-deploy"), the process is already ptrace()-d by debugserver. However,
// when an app is launched from the home screen, it is not, so for debug builds
// we initialize the ptrace() relationship via PT_TRACE_ME if necessary.
//
// Please see the following documents for more details:
//   - go/decommissioning-dbc
//   - go/decommissioning-dbc-engine
//   - go/decommissioning-dbc-tools

#include "flutter/runtime/ptrace_check.h"

#if TRACING_CHECKS_NECESSARY

#include <sys/sysctl.h>
#include <sys/types.h>

#include <mach/machine.h>
#include <mutex>

#include "flutter/fml/build_config.h"

// Being extra careful and adding additional landmines that will prevent
// compilation of this TU in an incorrect runtime mode.
static_assert(OS_IOS, "This translation unit is iOS specific.");
static_assert(FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG,
              "This translation unit must only be compiled in the debug "
              "runtime mode as it "
              "contains private API usage.");

#define PT_TRACE_ME 0
#define PT_SIGEXC 12
extern "C" int ptrace(int request, pid_t pid, caddr_t addr, int data);

namespace flutter {

static bool IsLaunchedByFlutterCLI(const Settings& vm_settings) {
  // Only the Flutter CLI passes "--enable-checked-mode". Therefore, if the flag
  // is present, we have been launched by "ios-deploy" via "debugserver".
  //
  // We choose this flag because it is always passed to launch debug builds.
  return vm_settings.enable_checked_mode;
}

static bool IsLaunchedByXcode() {
  // Use "sysctl()" to check if we're currently being debugged (e.g. by Xcode).
  // We could also check "getppid() != 1" (launchd), but this is more direct.
  const pid_t self = getpid();
  int mib[5] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, self, 0};

  auto proc = std::make_unique<struct kinfo_proc>();
  size_t proc_size = sizeof(struct kinfo_proc);
  if (::sysctl(mib, 4, proc.get(), &proc_size, nullptr, 0) < 0) {
    FML_LOG(ERROR) << "Could not execute sysctl() to get current process info: "
                   << strerror(errno);
    return false;
  }

  return proc->kp_proc.p_flag & P_TRACED;
}

static bool EnableTracingManually(const Settings& vm_settings) {
  if (::ptrace(PT_TRACE_ME, 0, nullptr, 0) == -1) {
    FML_LOG(ERROR) << "Could not call ptrace(PT_TRACE_ME): " << strerror(errno);
    // No use trying PT_SIGEXC -- it's only needed if PT_TRACE_ME succeeds.
    return false;
  }

  if (::ptrace(PT_SIGEXC, 0, nullptr, 0) == -1) {
    FML_LOG(ERROR) << "Could not call ptrace(PT_SIGEXC): " << strerror(errno);
    return false;
  }

  // The previous operation causes this process to not be reaped after it
  // terminates (even if PT_SIGEXC fails). Issue a warning to the console every
  // (approximiately) maxproc/10 leaks. See the links above for an explanation
  // of this issue.
  size_t maxproc = 0;
  size_t maxproc_size = sizeof(size_t);
  const int sysctl_result =
      ::sysctlbyname("kern.maxproc", &maxproc, &maxproc_size, nullptr, 0);
  if (sysctl_result < 0) {
    FML_LOG(ERROR)
        << "Could not execute sysctl() to determine process count limit: "
        << strerror(errno);
    return false;
  }

  const char* warning =
      "Launching a debug-mode app from the home screen may cause problems.\n"
      "Please compile a profile-/release-build, launch your app via \"flutter "
      "run\", or see https://github.com/flutter/flutter/wiki/"
      "PID-leak-in-iOS-debug-builds-launched-from-home-screen for details.";

  if (vm_settings.verbose_logging  // used for testing and also informative
      || sysctl_result < 0         // could not determine maximum process count
      || maxproc / 10 == 0         // avoid division (%) by 0
      || getpid() % (maxproc / 10) == 0)  // warning every ~maxproc/10 leaks
  {
    FML_LOG(ERROR) << warning;
  }

  return true;
}

static bool IsTracingCheckUnnecessaryOnVersion() {
#if !OS_IOS || TARGET_OS_SIMULATOR
  return true;
#endif

  // Check for arm64e.
  cpu_type_t cputype = 0;
  size_t cputype_size = sizeof(cpu_type_t);
  if (::sysctlbyname("hw.cputype", &cputype, &cputype_size, nullptr, 0) < 0) {
    FML_LOG(ERROR) << "Could not execute sysctl() to get CPU type: "
                   << strerror(errno);
  }

  cpu_subtype_t cpusubtype = 0;
  if (::sysctlbyname("hw.cpusubtype", &cpusubtype, &cputype_size, nullptr, 0) <
      0) {
    FML_LOG(ERROR) << "Could not execute sysctl() to get CPU subtype: "
                   << strerror(errno);
  }

  // Tracing is necessary unless the device is arm64e (A12 chip or higher).
  if (cputype != CPU_TYPE_ARM64 || cpusubtype != CPU_SUBTYPE_ARM64E) {
    return false;
  }

  // Check for iOS 14.2 and higher.
  size_t osversion_size;
  ::sysctlbyname("kern.osversion", NULL, &osversion_size, NULL, 0);
  char osversionBuffer[osversion_size];

  if (::sysctlbyname("kern.osversion", osversionBuffer, &osversion_size, NULL,
                     0) < 0) {
    FML_LOG(ERROR) << "Could not execute sysctl() to get current OS version: "
                   << strerror(errno);

    return false;
  }

  int major_version = 0;
  char minor_letter = 'Z';

  for (size_t index = 0; index < osversion_size; index++) {
    char version_char = osversionBuffer[index];
    // Find the minor version build letter.
    if (isalpha(version_char)) {
      major_version = atoi((const char*)osversionBuffer);
      minor_letter = toupper(version_char);
      break;
    }
  }
  // 18B92 is iOS 14.2 beta release candidate where tracing became unnecessary.
  return major_version > 18 || (major_version == 18 && minor_letter >= 'B');
}

static bool EnableTracingIfNecessaryOnce(const Settings& vm_settings) {
  if (IsTracingCheckUnnecessaryOnVersion()) {
    return true;
  }

  if (IsLaunchedByFlutterCLI(vm_settings)) {
    return true;
  }

  if (IsLaunchedByXcode()) {
    return true;
  }

  return EnableTracingManually(vm_settings);
}

static TracingResult sTracingResult = TracingResult::kNotAttempted;

bool EnableTracingIfNecessaryImpl(const Settings& vm_settings) {
  static std::once_flag tracing_flag;

  std::call_once(tracing_flag, [&vm_settings]() {
    sTracingResult = EnableTracingIfNecessaryOnce(vm_settings)
                         ? TracingResult::kEnabled
                         : TracingResult::kDisabled;
  });
  return sTracingResult != TracingResult::kDisabled;
}

TracingResult GetTracingResultImpl() {
  return sTracingResult;
}

}  // namespace flutter

#endif  // TRACING_CHECKS_NECESSARY
