# `package:scenario_app`

This package simulates a Flutter app that uses the engine (`dart:ui`) only. It
is used to test the engine in isolation from the rest of the Flutter framework
and tooling.

## Adding a New Scenario

Create a new subclass of [Scenario](src/scenario.dart) and add it to the map
in [scenarios.dart](src/scenarios.dart). For an example, see
[animated_color_square.dart](src/animated_color_square.dart), which draws a
continuously animating colored square that bounces off the sides of the
viewport.

Then set the scenario from the Android or iOS app by calling `set_scenario` on
platform channel `driver`.
