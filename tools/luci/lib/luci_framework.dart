// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// Provides the core concepts of the build framework, such as [Target] and
/// [TargetRunner].

// @dart = 2.6
import 'package:meta/meta.dart';
export 'package:meta/meta.dart';

/// The Linux LUCI build agent.
const List<String> kLinuxAgent = <String>['linux'];

/// The Mac LUCI build agent.
const List<String> kMacAgent = <String>['mac'];

/// The Windows LUCI build agent.
const List<String> kWindowsAgent = <String>['windows'];

/// A build or test target that can be run using `luci.dart run`.
@sealed
class Target {
  /// Creates a concrete build target.
  const Target({
    @required this.name,
    @required this.agentProfiles,
    @required this.runner,
    this.environment,
  });

  /// A unique name of the target.
  ///
  /// This name is used to identify this target and run it.
  final String name;

  /// Names of the agent profiles that this target can run on.
  ///
  /// Typically, this list contains one agent profile, but there can be
  /// use-cases for specifying multiple profiles, such as:
  ///
  /// - During a migration from an old profile to a new profile we may
  ///   temporarily specify both profiles, ramp up the new one, then remove
  ///   the old profile from this list, then finally sunset the old profile
  ///   agents.
  /// - When a target only cares about the host configuration and not the
  ///   devices. For example, you may need a Mac agent, but you don't care
  ///   if that agent has an Android device, iOS device, or no devices at all.
  ///   Then you can specify "mac-android", "mac-ios", and "mac".
  final List<String> agentProfiles;

  /// Runs this target.
  final TargetRunner runner;

  /// Additional environment variables used when running this target.
  final Map<String, String> environment;

  /// Serializes this target to JSON for digestion by the LUCI recipe.
  Map<String, dynamic> toJson() {
    return <String, dynamic>{
      'name': name,
      'agentProfiles': agentProfiles,
      'runner': runner.runtimeType.toString(),
      if (environment != null)
        'environment' : environment,
    };
  }
}

/// Runs LUCI targets.
///
/// Implement this class to provide
abstract class TargetRunner {
  // Prevents this class from being extended.
  factory TargetRunner._() => throw 'This class must be implemented, not extended.';

  /// Runs a single target given the target's description.
  Future<void> run(Target target);
}
