// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PLATFORM_COMMON_PLATFORM_PROVIDED_MENU_H_
#define PLATFORM_COMMON_PLATFORM_PROVIDED_MENU_H_

// Enumerates the provided menus that a platform may support.
// Must be kept in sync with the framework enum in widgets/menu.dart.
enum PlatformProvidedMenu {
  // orderFrontStandardAboutPanel macOS provided menu
  about,

  // terminate macOS provided menu
  quit,

  // Services macOS provided submenu.
  servicesSubmenu,

  // hide macOS provided menu
  hide,

  // hideOtherApplications macOS provided menu
  hideOtherApplications,

  // unhideAllApplications macOS provided menu
  showAllApplications,

  // startSpeaking macOS provided menu
  startSpeaking,

  // stopSpeaking macOS provided menu
  stopSpeaking,

  // toggleFullScreen macOS provided menu
  toggleFullScreen,

  // performMiniaturize macOS provided menu
  minimizeWindow,

  // performZoom macOS provided menu
  zoomWindow,

  // arrangeInFront macOS provided menu
  arrangeWindowInFront,
};

#endif  // PLATFORM_COMMON_PLATFORM_provided_MENU_H_
