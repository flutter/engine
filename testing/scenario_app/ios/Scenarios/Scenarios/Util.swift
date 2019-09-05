// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Foundation
import Flutter

class Util: NSObject {
  @objc static func runEngine(scenario: String, completion: (() -> Void)? = nil) -> FlutterEngine {
    let engine: FlutterEngine = FlutterEngine(name: "Test engine for \(scenario)", project: nil)
    engine.run(withEntrypoint: nil)
    engine.binaryMessenger.setMessageHandlerOnChannel(
      "scenario_status",
      binaryMessageHandler: {(message, reply) -> Void in
        completion?()
        engine.binaryMessenger.send(
          onChannel: "set_scenario",
          message: scenario.data(using: String.Encoding.utf8))})
    return engine
  }
}
