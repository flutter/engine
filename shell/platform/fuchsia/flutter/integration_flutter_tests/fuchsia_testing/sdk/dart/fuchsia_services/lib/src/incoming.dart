// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:io' as io;

import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';
import 'package:zircon/zircon.dart';

/// Helper class to connect to incoming services.
///
/// [Incoming] is used to connect to the services exposed by a launched component.
/// The general pattern for launching and connecting to a service is:
/// ```
/// final incoming = Incoming();
/// final launchInfo = LaunchInfo(
///   url: serverUrl,
///   directoryRequest: incoming.request().passChannel(),
/// );
///
/// final launcherProxy = LauncherProxy();
/// context.incoming.connectToService(launcherProxy);
///
/// launcherProxy.createComponent(launchInfo, ctrl);
/// final myProxy = MyProxy();
/// incoming.connectToService(myProxy);
///
/// incoming.close();
/// ```
///
/// These services have been offered to this component by its parent or are
/// ambiently offered by the Component Framework.
class Incoming {
  static const String _serviceRootPath = '/svc';
  final DirectoryProxy _dirProxy;

  /// Initializes [Incoming] with an unbound [DirectoryProxy] which can be used
  /// to bind to a launched component's services.
  Incoming() : this.withDirectory(DirectoryProxy());

  /// Initializes [Incoming] with a [Directory] that should be bound to `/svc`
  /// of this component.
  ///
  /// If you are launching a component use the [Incoming()] constructor
  /// to get an unbound directory.
  Incoming.withDirectory(this._dirProxy);

  /// Initializes [Incoming] with a [Directory] that is bound to `/svc` of this
  /// component.
  ///
  /// If you are launching a component use the [Incoming()] constructor
  /// to get an unbound directory.
  factory Incoming.fromSvcPath() {
    if (!io.Directory(_serviceRootPath).existsSync()) {
      final componentName = io.Platform.script.pathSegments
          .lastWhere((_) => true, orElse: () => '???');
      throw Exception(
          'Attempting to connect to the /svc directory for [$componentName] failed. '
          'This is an indication that the system is not in a valid state.');
    }
    final channel = Channel.fromFile(_serviceRootPath);
    final directory = DirectoryProxy()
      ..ctrl.bind(InterfaceHandle<Directory>(channel));
    return Incoming.withDirectory(directory);
  }

  /// Terminates connection and return Zircon status.
  Future<int> close() async {
    final status = await _dirProxy.close();
    _dirProxy.ctrl.close();
    return status;
  }

  /// Connects to the incoming service specified by [serviceProxy].
  ///
  /// If this object is not bound via the [request] method before
  /// this method is called an [IncomingStateException] will be thrown.
  void connectToService<T>(AsyncProxy<T>? serviceProxy) {
    if (serviceProxy == null) {
      throw ArgumentError.notNull('serviceProxy');
    }
    final String? serviceName = serviceProxy.ctrl.$serviceName;
    if (serviceName == null) {
      throw Exception(
          "${serviceProxy.ctrl.$interfaceName}'s controller.\$serviceName must "
          'not be null. Check the FIDL file for a missing [Discoverable]');
    }

    // Creates an interface request and binds one of the channels. Binding this
    // channel prior to connecting to the agent allows the developer to make
    // proxy calls without awaiting for the connection to actually establish.
    final serviceProxyRequest = serviceProxy.ctrl.request();

    connectToServiceByNameWithChannel(
        serviceName, serviceProxyRequest.passChannel());
  }

  /// Connects to the incoming service specified by [serviceName] through the
  /// [channel] endpoint supplied by the caller.
  ///
  /// If the service provider is not willing or able to provide the requested
  /// service, it should close the [channel].
  ///
  /// If this object is not bound via the [request] method before
  /// this method is called an [IncomingStateException] will be thrown.
  void connectToServiceByNameWithChannel(
      String? serviceName, Channel? channel) {
    if (serviceName == null) {
      throw Exception(
          'serviceName must not be null. Check the FIDL file for a missing '
          '[Discoverable]');
    }
    if (channel == null) {
      throw ArgumentError.notNull('channel');
    }

    if (_dirProxy.ctrl.isUnbound) {
      throw IncomingStateException(
          'The directory must be bound before trying to connect to a service. '
          'See [Incoming.request] for more information');
    }

    // connection flags for service: can read & write from target object.
    const int _openFlags = openRightReadable | openRightWritable;
    // 0755
    const int _openMode = 0x1ED;

    _dirProxy.open(
        _openFlags, _openMode, serviceName, InterfaceRequest<Node>(channel));
  }

  /// Connects to the incoming service specified by [serviceProxy] through the
  /// [channel] endpoint supplied by the caller.
  ///
  /// If the service provider is not willing or able to provide the requested
  /// service, it should close the [channel].
  ///
  /// If this object is not bound via the [request] method before
  /// this method is called an [IncomingStateException] will be thrown.
  void connectToServiceWithChannel<T>(
      AsyncProxy<T>? serviceProxy, Channel? channel) {
    if (serviceProxy == null) {
      throw ArgumentError.notNull('serviceProxy');
    }
    if (channel == null) {
      throw ArgumentError.notNull('channel');
    }
    final String? serviceName = serviceProxy.ctrl.$serviceName;
    if (serviceName == null) {
      throw Exception(
          "${serviceProxy.ctrl.$interfaceName}'s controller.\$serviceName must "
          'not be null. Check the FIDL file for a missing [Discoverable]');
    }
    connectToServiceByNameWithChannel(serviceName, channel);
  }

  /// Takes ownership of the Directory's request object for binding
  /// to another processes outgoing services.
  ///
  /// The returned [InterfaceRequest] object is suitable to pass to
  /// an object which can bind the directory.
  ///
  /// Subsequent calls to this method throw a FidlStateException
  InterfaceRequest<Node> request() =>
      InterfaceRequest(_dirProxy.ctrl.request().passChannel());
}

/// An Exception that can be thrown if the [Incoming] object is in
/// a bad state.
class IncomingStateException implements Exception {
  /// An error message describing the reason for this exception.
  final String message;

  /// Creates a new instance of this.
  IncomingStateException(this.message);

  @override
  String toString() => 'IncomingStateException: $message';
}
