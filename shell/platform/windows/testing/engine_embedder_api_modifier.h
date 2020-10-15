// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/flutter_windows_engine.h"

namespace flutter {

// A test utility class providing the ability to access and alter the embedder
// API proc table for an engine instance.
class EngineEmbedderApiModifier {
 public:
  explicit EngineEmbedderApiModifier(FlutterWindowsEngine* engine)
      : engine_(engine) {}

  // Returns the engine's embedder API proc table, allowing for modification.
  FlutterEngineProcTable& embedder_api() { return engine_->embedder_api_; }

 private:
  FlutterWindowsEngine* engine_;
};

}  // namespace flutter
