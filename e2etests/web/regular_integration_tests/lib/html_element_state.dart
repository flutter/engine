import 'dart:html';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';

// ignore: public_member_api_docs
class HtmlWidgetController {
  DivElement _wrapperDiv;
  DivElement _scrollableDiv;

  // ignore: public_member_api_docs
  void registerFactory() {
    ui.platformViewRegistry.registerViewFactory('ScrollableElement', (int viewId) {
      _wrapperDiv = DivElement()
        ..id = 'wrapper'
        ..style.overflowY = 'auto';
      _scrollableDiv = DivElement()
        ..id = 'scrollable'
        ..style.width = '100px'
        ..style.height = '500px'
        ..style.backgroundImage = 'linear-gradient(red, orange, yellow, green, blue, indigo, violet)';
      _wrapperDiv.append(_scrollableDiv);
      return _wrapperDiv;
    });
  }

  // ignore: public_member_api_docs
  void scrollDown(int scrollPosition) {
    _wrapperDiv.scrollTop = scrollPosition;
  }

  // ignore: public_member_api_docs
  int getScrollPos() {
    return _wrapperDiv.scrollTop;
  }
}

// ignore: public_member_api_docs
final HtmlWidgetController htmlWidgetController = HtmlWidgetController();

void main() {
  htmlWidgetController.registerFactory();
  runApp(MaterialApp(home: _HomeScreen()));
}

class _HomeScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      drawer: Drawer(
          child: RaisedButton(
        key: const Key('closeDrawer'),
        child: const Text('Close'),
        onPressed: () => Navigator.of(context).pop(),
      )),
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
          child: const Text('Open'),
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
