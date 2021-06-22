// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';

import 'incoming.dart';
import 'outgoing.dart';

/// Context information that this component received at startup.
///
/// The [ComponentContext] holds references to the services and connections that
/// the component was launched with. Authors can use the component context to
/// access useful information for connecting to other components and interacting
/// with the framework.
class ComponentContext {
  static ComponentContext? _componentContext;

  /// Services that are available to this component.
  ///
  /// These services have been offered to this component by its parent or are
  /// ambiently offered by the Component Framework.
  final Incoming svc;

  /// Services and data exposed to other components.
  ///
  /// Use [outgoing] to publish services and data to the component manager and
  /// other components.
  final Outgoing outgoing;

  /// This constructor is used by the modular test harness, but should not be
  /// used elsewhere. [ComponentContext.create()] and
  /// [ComponentContext.createAndServe()] should typically be used instead.
  ComponentContext({
    required this.svc,
    required this.outgoing,
  });

  /// Creates the component context. Users need to make sure that they call
  /// serve after they have finished adding all their public services.
  ///
  /// Note: ComponentContext can only be constructed once.
  ///
  /// Example:
  ///
  /// ```
  /// Future<void> main() async {
  ///   final context = ComponentContext.create();
  ///   ...
  ///   await doAsyncSetup();
  ///   ...
  ///   context.outgoing.serveFromStartupInfo();
  /// }
  /// ```
  factory ComponentContext.create() {
    if (_componentContext != null) {
      throw Exception(
          'Attempted to construct ComponentContext multiple times. Ensure that the existing ComponentContext is reused instead of constructing multiple instances.');
    }
    return _componentContext = ComponentContext(
      svc: Platform.isFuchsia ? Incoming.fromSvcPath() : Incoming(),
      outgoing: Outgoing(),
    );
  }

  /// Creates a ComponentContext and immediately serves the outgoing. This
  /// method is useful for simple programs who do not have to do any complicated
  /// setup.
  ///
  /// Note: ComponentContext can only be constructed once.
  ///
  /// Example:
  ///
  /// ```
  /// void main() {
  ///   final context = ComponentContext.createAndServe();
  ///   ...
  /// }
  /// ```
  factory ComponentContext.createAndServe() {
    return ComponentContext.create()..outgoing.serveFromStartupInfo();
  }
}
