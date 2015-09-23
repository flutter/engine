// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//import 'package:mojo/mojo/url_response.mojom.dart';
//import 'package:sky/material.dart';
//import 'package:sky/rendering.dart';
import 'package:sky/services.dart';
//import 'package:sky/widgets.dart';

class UpdateTask {
  UpdateTask() {}

  String toString() => "UpdateTask()";
}

// parse local manifest; get update_url
// fetch remote manifest
// compare versions
// download new app.skyx
// replace old app.skyx
// notify caller we're done
//
// need:
// local data_dir (for manifest)
// local tmp_dir (for downloads)
// method for notifying caller. native? mojo?

runTest() async {
  print("Fetching...");
//  String data = await fetchString("http://mpcomplete.org");
//  print("fetched: $data");
}

void main() {
  var x = new UpdateTask();
  print("Success: $x");
  runTest();
}
