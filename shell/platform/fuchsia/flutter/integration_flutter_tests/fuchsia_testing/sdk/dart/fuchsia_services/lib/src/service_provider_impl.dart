// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_sys/fidl_async.dart' as fidl;
import 'package:zircon/zircon.dart';

/// The [ServiceProviderImpl] is a concrete implementation of the
/// [fidl.ServiceProvider] interface.
//  TODO(fxbug.dev/16108, fxbug.dev/15903): Move to fuchsia_modular and make private
class ServiceProviderImpl extends fidl.ServiceProvider {
  final _binding = fidl.ServiceProviderBinding();

  final Map<String, void Function(Channel?)> _connectorThunks = {};

  /// Registers the [connector] function with the given [serviceName]. The
  /// [connector] function is invoked when the service provider is asked by
  /// the framework to connect to the service in the [connectToService] method.
  void addServiceForName<T>(
      void Function(InterfaceRequest<T>) connector, String serviceName) {
    _connectorThunks[serviceName] = (Channel? channel) {
      connector(InterfaceRequest<T>(channel));
    };
  }

  /// Binds this object to the [interfaceRequest].
  void bind(InterfaceRequest<fidl.ServiceProvider> interfaceRequest) {
    _binding.bind(this, interfaceRequest);
  }

  /// Closes the connection to the underlying binding.
  void close() {
    _binding.close();
  }

  /// See [fidl.ServiceProvider#connectToService].
  @override
  Future<void> connectToService(String serviceName, Channel? channel) async {
    final connectorThunk = _connectorThunks[serviceName];
    if (connectorThunk != null) {
      connectorThunk(channel);
    } else {
      channel!.close();
    }
  }
}
