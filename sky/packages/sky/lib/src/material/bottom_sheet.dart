// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/animation.dart';
import 'package:flutter/painting.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';

import 'colors.dart';
import 'material.dart';

const Duration _kBottomSheetDuration = const Duration(milliseconds: 200);
const double _kMinFlingVelocity = 700.0;
const double _kFlingVelocityScale = 1.0 / 300.0;

class _ModalBottomSheet extends StatefulComponent {
  _ModalBottomSheet({ Key key, this.route }) : super(key: key);

  final _ModalBottomSheetRoute route;

  _ModalBottomSheetState createState() => new _ModalBottomSheetState();
}

class _ModalBottomSheetLayout extends OneChildLayoutDelegate {
  // The distance from the bottom of the parent to the top of the BottomSheet child.
  AnimatedValue<double> childTop = new AnimatedValue<double>(0.0);

  BoxConstraints getConstraintsForChild(BoxConstraints constraints) {
    return new BoxConstraints(
      minWidth: constraints.maxWidth,
      maxWidth: constraints.maxWidth,
      minHeight: 0.0,
      maxHeight: constraints.maxHeight * 9.0 / 16.0
    );
  }

  Point getPositionForChild(Size size, Size childSize) {
    childTop.end = childSize.height;
    return new Point(0.0, size.height - childTop.value);
  }
}

abstract class _BottomSheetDragMixin {
  bool _dragEnabled = false;

  Performance get _performance;

  void _handleDragStart(Point position) {
    _dragEnabled = !_performance.isAnimating;
  }

  double _dragProgress(double delta);

  void _handleDragUpdate(double delta) {
    if (!_dragEnabled)
      return;
    _performance.progress -= _dragProgress(delta);
  }

  void _handleDragEnd(Offset velocity, BuildContext context) {
    if (!_dragEnabled)
      return;
    if (velocity.dy > _kMinFlingVelocity)
      _performance.fling(velocity: -velocity.dy * _kFlingVelocityScale).then((dynamic value) {
        Navigator.of(context).pop(value);
      });
    else
      _performance.forward();
  }
}

class _ModalBottomSheetState extends State<_ModalBottomSheet> with _BottomSheetDragMixin {

  Performance get _performance => config.route._performance;
  double _dragProgress(double delta) => delta / _layout.childTop.end;

  final _ModalBottomSheetLayout _layout = new _ModalBottomSheetLayout();
  Widget build(BuildContext context) {
    return new GestureDetector(
      onTap: () { Navigator.of(context).pop(); },
      child: new BuilderTransition(
        performance: config.route._performance,
        variables: <AnimatedValue<double>>[_layout.childTop],
        builder: (BuildContext context) {
          return new ClipRect(
            child: new CustomOneChildLayout(
              delegate: _layout,
              token: _layout.childTop.value,
              child: new GestureDetector(
                onVerticalDragStart: _handleDragStart,
                onVerticalDragUpdate: _handleDragUpdate,
                onVerticalDragEnd: (Offset velocity) { _handleDragEnd(velocity, context); },
                child: new Material(child: config.route.child)
              )
            )
          );
        }
      )
    );
  }
}

class _ModalBottomSheetRoute extends ModalRoute {
  _ModalBottomSheetRoute({ this.completer, this.child });

  final Completer completer;
  final Widget child;

  bool get opaque => false;
  Duration get transitionDuration => _kBottomSheetDuration;

  Performance _performance;

  Performance createPerformance() {
    _performance = super.createPerformance();
    return _performance;
  }

  Color get barrierColor => Colors.black54;
  Widget buildModalWidget(BuildContext context) => new _ModalBottomSheet(route: this);

  void didPop([dynamic result]) {
    completer.complete(result);
    super.didPop(result);
  }
}

Future showModalBottomSheet({ BuildContext context, Widget child }) {
  assert(child != null);
  final Completer completer = new Completer();
  Navigator.of(context).pushEphemeral(new _ModalBottomSheetRoute(
    completer: completer,
    child: child
  ));
  return completer.future;
}

class _PersistentBottomSheet extends StatefulComponent {
  _PersistentBottomSheet({ Key key, this.route }) : super(key: key);

  final _PersistentBottomSheetRoute route;

  _PersistentBottomSheetState createState() => new _PersistentBottomSheetState();
}

class _PersistentBottomSheetState extends State<_PersistentBottomSheet> with _BottomSheetDragMixin {

  Size _childSize;

  void _updateChildSize(Size newSize) {
    setState(() {
      _childSize = newSize;
    });
  }

  Performance get _performance => config.route._performance;
  double _dragProgress(double delta) => delta / (_childSize?.height ?? delta);

  Widget build(BuildContext context) {
    return new AlignTransition(
      performance: config.route.performance,
      alignment: new AnimatedValue<FractionalOffset>(const FractionalOffset(0.0, 0.0)),
      heightFactor: new AnimatedValue<double>(0.0, end: 1.0),
      child: new GestureDetector(
        onVerticalDragStart: _handleDragStart,
        onVerticalDragUpdate: _handleDragUpdate,
        onVerticalDragEnd: (Offset velocity) { _handleDragEnd(velocity, context); },
        child: new Material(
          child: new SizeObserver(child: config.route.child, onSizeChanged: _updateChildSize))
      )
    );
  }
}

class _PersistentBottomSheetRoute extends TransitionRoute {
  _PersistentBottomSheetRoute({ this.completer, this.child });

  final Widget child;
  final Completer completer;

  bool get opaque => false;
  Duration get transitionDuration => _kBottomSheetDuration;

  Performance _performance;
  Performance createPerformance() {
    _performance = super.createPerformance();
    return _performance;
  }

  void didPop([dynamic result]) {
    completer.complete(result);
    super.didPop(result);
  }
}

Future showBottomSheet({ BuildContext context, GlobalKey<PlaceholderState> placeholderKey, Widget child }) {
  assert(child != null);
  assert(placeholderKey != null);
  final Completer completer = new Completer();
  _PersistentBottomSheetRoute route = new _PersistentBottomSheetRoute(child: child, completer: completer);
  placeholderKey.currentState.child = new _PersistentBottomSheet(route: route);
  Navigator.of(context).pushEphemeral(route);
  return completer.future;
}
