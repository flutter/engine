// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// A collection of textures available to the engine.
abstract class TextureRegistry {
  /// Registers a texture, returns an id that can be used to reference that
  /// texture in subsequent calls.
  int registerTexture(Object source);

  /// Unregisters a texture that was previously registered with
  /// [registerTexture].
  void unregisterTexture(int id);

  /// Notifies Flutter that the content of the previously registered texture
  /// has been updated. [id] is the id of the texture that was previously
  /// returned by [registerTexture].
  void textureFrameAvailable(int id);
}
