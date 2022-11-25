// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// A collection of textures available to the engine.
abstract class TextureRegistry {
  /// Registers a texture.
  int registerTexture(Object source);

  /// Unregisters the texture with the given id.
  void unregisterTexture(int id);

  /// Notifies Flutter that the content of the previously registered texture
  /// has been updated.
  void textureFrameAvailable(int id);
}
