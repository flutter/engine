// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import Flutter
import UIKit

class ScreenBeforeFlutter: UIViewController {
  @objc init(completion: (() -> Void)? = nil) {
    self.engine = Util.runEngine(scenario: "poppable_screen", completion: completion)
    super.init(nibName: nil, bundle: nil)
  }

  required init?(coder aDecoder: NSCoder) {
    self.engine = Util.runEngine(scenario: "poppable_screen")
    super.init(coder: aDecoder)
  }

  private lazy var showFlutterButton: UIButton = {
    let button = UIButton(type: .system)
    button.translatesAutoresizingMaskIntoConstraints = false
    button.backgroundColor = .blue
    button.setTitle("Show Flutter", for: .normal)
    button.tintColor = .white
    button.clipsToBounds = true
    button.addTarget(self, action: #selector(self.showFlutter), for: .touchUpInside)
    return button
  }()

  var engine: FlutterEngine

  override func viewDidLoad() {
    super.viewDidLoad()
    view.backgroundColor = .gray

    view.addSubview(showFlutterButton)
    NSLayoutConstraint.activate([
      showFlutterButton.centerXAnchor.constraint(equalTo:self.view.centerXAnchor),
      showFlutterButton.centerYAnchor.constraint(equalTo:self.view.centerYAnchor),
      showFlutterButton.heightAnchor.constraint(equalToConstant:50),
      showFlutterButton.widthAnchor.constraint(equalToConstant:150),
    ])

    engine.run(withEntrypoint: nil)
  }

  @objc func showFlutter() -> FlutterViewController {
    let flutterVC: FlutterViewController =
        FlutterViewController(engine: engine, nibName: nil, bundle: nil)
    present(
      flutterVC,
      animated: false,
      completion: nil)
    return flutterVC
  }
}
