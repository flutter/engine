abstract class TextureRegistry {
  /// Registers a texture.
  int registerTexture(Object source);

  /// Unregisters the texture with the given id.
  void unregisterTexture(int id);

  /// Notifies Flutter that the content of the previously registered texture
  /// has been updated.
  void textureFrameAvailable(int id);
}
