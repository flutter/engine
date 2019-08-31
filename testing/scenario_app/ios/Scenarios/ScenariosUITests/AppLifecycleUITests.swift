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
    let rootVC = UIApplication.shared.keyWindow!.rootViewController as! ScreenBeforeFlutter
    let engine = rootVC.engine
    
    var lastLifecycleEvent:Bool
    engine.lifecycleChannel.setMessageHandler({
      (call:FlutterMethodCall, result:FlutterResult) -> Void {
        
      }
    })
  }
}
