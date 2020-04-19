import 'dart:html';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';

void main() {
  ui.platformViewRegistry.registerViewFactory('ScrollableElement', (int viewId) {
    final DivElement wrapperDiv = DivElement()
      ..id = 'wrapper'
      ..style.overflowY = 'auto';
    final DivElement scrollableDiv = DivElement()
      ..id = 'scrollable'
      ..style.width = '100px'
      ..style.height = '500px'
      ..style.backgroundImage = 'linear-gradient(red, orange, yellow, green, blue, indigo, violet)';
    wrapperDiv.append(scrollableDiv);
    return wrapperDiv;
  });

  runApp(MaterialApp(home: HomeScreen()));
}

class HomeScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      drawer: Drawer(
        child: RaisedButton(
          key: const Key('closeDrawer'),
          child: const Text('Close'),
          onPressed: () => Navigator.of(context).pop(),
        )
      ),
      body: Center(
        child: Container(
          width: 100,
          height: 100,
          child: _HtmlWidget(),
        ),
      ),
      floatingActionButton: Builder(builder: (BuildContext context) {
        return FloatingActionButton(
          key: const Key('openDrawer'),
          child: const Icon(Icons.menu, color: Colors.white),
          onPressed: () {
            Scaffold.of(context).openDrawer();
          },
        );
      }),
    );
  }
}

class _HtmlWidget extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return const HtmlElementView(viewType: 'ScrollableElement');
  }
}
