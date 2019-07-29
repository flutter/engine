#ifndef FLUTTER_RUNTIME_PTRACE_IOS_H_
#define FLUTTER_RUNTIME_PTRACE_IOS_H_

#include "flutter/common/settings.h"

#if OS_IOS && (FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG)

// Ensure that the current process is or was ptrace()-d at some point in its
// life. Can only be used within debug builds for iOS.
void EnsureDebuggedIOS(const flutter::Settings& vm_settings);

#endif

#endif  // FLUTTER_RUNTIME_PTRACE_IOS_H_
