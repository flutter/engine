// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Flutter
import XCTest

@testable import Scenarios

// This test makes sure that the iOS embedding generates the expected sets of
// AppLifecycleState events through the engine's lifecycle channel.
//
// This is an iOS only test because the Android embedding only connects to the
// lifecycle of one UI component attached to the engine rather than potentially
// the AppDelegate and the ViewController. Only iOS might conflate the 2
// lifecycle sources.
class AppLifecycleTests: XCTestCase {
  override func setUp() {
    continueAfterFailure = false
  }

  func testLifecycleChannel() {
    let engineStartedExpectation = self.expectation(description: "Engine started");

    // Let the engine finish booting (at the end of which the channels are properly set-up) before
    // moving onto the next step of showing the next view controller.
    let rootVC = ScreenBeforeFlutter(completion:{() -> Void in
      engineStartedExpectation.fulfill()
    })
    wait(for: [engineStartedExpectation], timeout: 5, enforceOrder: true)

    UIApplication.shared.keyWindow!.rootViewController = rootVC
    let engine = rootVC.engine

    var lifecycleExpectations:[XCTestExpectation] = []
    var lifecycleEvents:[String] = []

    lifecycleExpectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.inactive"))
    lifecycleExpectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.resumed"))

    var flutterVC = rootVC.showFlutter()
    engine.lifecycleChannel.setMessageHandler({
      (message, reply) -> Void in
      lifecycleEvents.append(message as! String)
      lifecycleExpectations.removeFirst().fulfill()
    })

    wait(for: lifecycleExpectations, timeout: 5, enforceOrder: true)
    // Expected sequence from showing the FlutterViewController is inactive and resumed.
    XCTAssertEqual(lifecycleEvents, ["AppLifecycleState.inactive", "AppLifecycleState.resumed"])

    // Now dismiss the FlutterViewController again and expect another inactive and paused.
    lifecycleExpectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.inactive"))
    lifecycleExpectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.paused"))
    flutterVC.dismiss(animated: false, completion: nil)
    wait(for: lifecycleExpectations, timeout: 5, enforceOrder: true)
    XCTAssertEqual(lifecycleEvents, ["AppLifecycleState.inactive", "AppLifecycleState.resumed",
                                     "AppLifecycleState.inactive", "AppLifecycleState.paused"])

    // Now put the app in the background (while the engine is still running) and bring it back to
    // the foreground. Granted, we're not winning any awards for hyper-realism but at least we're
    // checking that we aren't observing the UIApplication notifications and double registering
    // for AppLifecycleState events.
    UIApplication.shared.delegate?.applicationWillResignActive?(UIApplication.shared)
    NotificationCenter.default.post(name: UIApplication.willResignActiveNotification, object: nil)
    UIApplication.shared.delegate?.applicationDidEnterBackground?(UIApplication.shared)
    NotificationCenter.default.post(name: UIApplication.didEnterBackgroundNotification, object: nil)
    UIApplication.shared.delegate?.applicationWillEnterForeground?(UIApplication.shared)
    NotificationCenter.default.post(name: UIApplication.willEnterForegroundNotification, object: nil)
    UIApplication.shared.delegate?.applicationDidBecomeActive?(UIApplication.shared)
    NotificationCenter.default.post(name: UIApplication.didBecomeActiveNotification, object: nil)

    // There's no timing latch for our semi-fake background-foreground cycle so launch the
    // FlutterViewController again to check the complete event list again.
    lifecycleExpectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.inactive"))
    lifecycleExpectations.append(XCTestExpectation(
      description: "A loading FlutterViewController goes through AppLifecycleState.resumed"))
    flutterVC = rootVC.showFlutter()
    wait(for: lifecycleExpectations, timeout: 5, enforceOrder: true)
    XCTAssertEqual(lifecycleEvents, ["AppLifecycleState.inactive", "AppLifecycleState.resumed",
                                     "AppLifecycleState.inactive", "AppLifecycleState.paused",
                                     // We only added 2 from re-launching the FlutterViewController
                                     // and none from the background-foreground cycle.
                                     "AppLifecycleState.inactive", "AppLifecycleState.resumed"])
  }
}
