// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

export const baseUri = getBaseURI();

function getBaseURI() {
  const base = document.querySelector("base");
  return (base && base.getAttribute("href")) || "";
}

export function joinPathSegments(...segments) {
  return segments.map((segment, i) => {
    if (!segment) {
      return "";
    }

    if (i === 0) {
      return stripRightSlash(segment);
    } else {
      return stripLeftSlash(stripRightSlash(segment));
    }
  }).filter(x => x.length).join('/')
}

function stripLeftSlash(s) {
  return s.startsWith('/') ? s.substring(1) : s;
}

function stripRightSlash(s) {
  return s.endsWith('/') ? s.substring(0, s.length - 1) : s;
}

/**
 * Calculates the proper base URL for CanvasKit/Skwasm assets.
 * 
 * @param {import("./types").FlutterConfiguration} config
 * @param {import("./types").BuildConfig} buildConfig
 */
export function getDefaultCanvaskitBaseUrl(config, buildConfig) {
  if (config.canvasKitBaseUrl) {
    return config.canvasKitBaseUrl;
  }
  if (buildConfig.engineRevision && !buildConfig.useLocalCanvasKit) {
    return joinPathSegments('https://www.gstatic.com/flutter-canvaskit', buildConfig.engineRevision);
  }
  return '/canvaskit';
}
