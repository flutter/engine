// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Flutter
import XCTest

@testable import Scenarios

class AppLifecycleUITests: XCTestCase {
  override func setUp() {
    continueAfterFailure = false
    
    let application = XCUIApplication()
    application.launchArguments = ["--screen-before-flutter"]
    application.launch()
  }

  func testLifecycleChannel() {
//    let rootVC = UIApplication.shared.keyWindow!.rootViewController as! ScreenBeforeFlutter
//    let engine = rootVC.engine
//
//    var expectations:[XCTestExpectation] = []
//    var lifecycleEvents:[String] = []
//    engine.lifecycleChannel.setMessageHandler({
//      (message, reply) -> Void in
//      lifecycleEvents.append(message as! String)
//      expectations.removeFirst().fulfill()
//    })
//
//    // Connecting the message handler after the engine started doesn't do anything.
//    XCTAssertEqual(lifecycleEvents.count, 0)
//
//    expectations.append(XCTestExpectation(
//      description: "A loading FlutterViewController goes through AppLifecycleState.inactive"))
//    expectations.append(XCTestExpectation(
//      description: "A loading FlutterViewController goes through AppLifecycleState.resumed"))
//    XCTAssertEqual(lifecycleEvents, ["AppLifecycleState.inactive", "AppLifecycleState.resumed"])
  }
}
