// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
// import 'package:flutter/material.dart';

void main(List<String> args) async {
  // final app = MaterialApp(home: TestApp(), debugShowCheckedModeBanner: false);
  // runApp(app);
}

// See https://medium.com/icnh/flutter-without-flutter-15177c91d066
// Reacting to Touch Event

// class TestApp extends StatelessWidget {
//   static const _yellow = Color.fromARGB(255, 255, 255, 0);
//   static const _pink = Color.fromARGB(255, 255, 0, 255);

//   final _backgroundColor = ValueNotifier(_pink);

//   @override
//   Widget build(BuildContext context) {
//     return Listener(
//       onPointerDown: (_) => _backgroundColor.value = _yellow,
//       child: AnimatedBuilder(
//           animation: _backgroundColor,
//           builder: (context, _) {
//             return Container(
//               color: _backgroundColor.value,
//             );
//           }),
//     );
//   }
// }
