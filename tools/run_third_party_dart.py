# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import os

if __name__ == '__main__':
    pub = "src/third_party/dart/tools/sdks/dart-sdk/bin/pub"
    if os.name == "nt":
        pub = "src/third_party/dart/tools/sdks/dart-sdk/bin/pub.bat"
    subprocess.call([pub, "global", "activate", "-spath", "./src/flutter/tools/generate_package_config"])
    subprocess.call([pub, "global", "run", "generate_package_config:generate_from_legacy", "src/flutter/flutter_frontend_server/.packages"])
    subprocess.call([pub, "global", "run", "generate_package_config:generate_from_legacy", "src/flutter/tools/const_finder/.packages"])
