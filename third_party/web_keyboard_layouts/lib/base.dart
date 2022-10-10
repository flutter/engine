// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

part of web_keyboard_layouts;

enum LayoutPlatform {
  win,
  linux,
  darwin,
}

const kDeadKey = 0x1000000;

class LayoutInfo {
  const LayoutInfo({
    required this.name,
    required this.platform,
    required this.mapping,
  });

  final String name;
  final LayoutPlatform platform;
  // Each element of `mapping` is a list of four:
  // noModifier, withShift, withAlt, withShiftAlt.
  // Each value is either 0xYYYYYY, or 0x1000000 for a dead key.
  final List<List<int>> mapping;
}
