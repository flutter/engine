// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/display_list_debugger.h"

#include <atomic>

#include "flutter/fml/logging.h"

namespace {
std::atomic<char*> saveDisplayListPath;
}

namespace flutter {
void DisplayListDebugger::HandleMessage(
    std::unique_ptr<PlatformMessage> message) {
  const fml::MallocMapping& data = message->data();
  char* old_path = saveDisplayListPath.exchange(
      strdup(reinterpret_cast<const char*>(data.GetMapping())));
  if (old_path) {
    free(old_path);
  }
}

void DisplayListDebugger::SaveDisplayList(
    const sk_sp<DisplayList>& display_list) {
  if (char* path = saveDisplayListPath.exchange(nullptr)) {
    fml::UniqueFD dir = fml::OpenDirectory(path, /*create_if_necessary=*/false,
                                           fml::FilePermission::kWrite);
    // DisplayList doesn't have an accessor for the byte size of the storage,
    // adding one may confuse the api so we derive it with bytes().
    size_t size = display_list->bytes(/*nested=*/false) - sizeof(DisplayList);
    fml::NonOwnedMapping mapping(display_list->GetStorage().get(), size);
    bool success = fml::WriteAtomically(dir, "display_list.dat", mapping);
    FML_LOG(ERROR) << "store display_list (" << success << "):" << path;
    free(path);
  }
}
}  // namespace flutter
