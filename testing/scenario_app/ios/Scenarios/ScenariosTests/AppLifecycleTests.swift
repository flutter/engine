// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Flutter
import XCTest

@testable import Scenarios

class AppLifecycleTests: XCTestCase {
  override func setUp() {
    continueAfterFailure = false
  }
  
  func testLifecycleChannel() {
    let rootVC = ScreenBeforeFlutter()
    UIApplication.shared.keyWindow!.rootViewController = rootVC
    let engine = rootVC.engine
    
    var expectations:[XCTestExpectation] = []
    var lifecycleEvents:[String] = []
    engine.lifecycleChannel.setMessageHandler({
      (message, reply) -> Void in
      lifecycleEvents.append(message as! String)
      expectations.removeFirst().fulfill()
    })
    
    // Connecting the message handler after the engine started doesn't do anything.
    XCTAssertEqual(lifecycleEvents.count, 0)
    
    expectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.inactive"))
    expectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.resumed"))
    
    rootVC.showFlutter()
    
    wait(for: expectations, timeout: 30, enforceOrder: true)
    XCTAssertEqual(lifecycleEvents, ["AppLifecycleState.inactive", "AppLifecycleState.resumed"])
  }
}
