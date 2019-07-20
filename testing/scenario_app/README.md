# Scenario App

This folder contains a dart:ui application and scripts to compile it to AOT
for exercising embedders.

It intentionally has no dependencies on the Flutter framework or tooling.

To add a new scenario, create a new subclass of `Scenario` and add it to the
map in `main.dart`.
