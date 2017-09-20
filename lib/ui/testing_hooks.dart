part of dart.ui;

/// @nodoc
/// Makes some internal functions visible for internal Flutter testing.
///
/// Do not use for non-testing purposes, or catastrophic problems may (and
/// probably will) occur.  These APIs are not part of the Flutter API, and are
/// subject to change.  Do not depend upon their existence.
class TestingHooks {
  /// @nodoc
  /// Make updateWindowMetrics hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void updateWindowMetrics(double devicePixelRatio, double width,
      double height, double top, double right, double bottom, double left) {
    _updateWindowMetrics(
        devicePixelRatio, width, height, top, right, bottom, left);
  }

  /// @nodoc
  /// Make updateLocale hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void updateLocale(String languageCode, String countryCode) =>
      _updateLocale(languageCode, countryCode);

  /// @nodoc
  /// Make updateTextScaleFactor hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void updateTextScaleFactor(double textScaleFactor) =>
      _updateTextScaleFactor(textScaleFactor);

  /// @nodoc
  /// Make updateSemanticsEnabled hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void updateSemanticsEnabled(bool enabled) =>
      _updateSemanticsEnabled(enabled);

  /// @nodoc
  /// Make dispatchPlatformMessage hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void dispatchPlatformMessage(
          String name, ByteData data, int responseId) =>
      _dispatchPlatformMessage(name, data, responseId);

  /// @nodoc
  /// Make dispatchPointerDataPacket hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void dispatchPointerDataPacket(ByteData packet) =>
      _dispatchPointerDataPacket(packet);

  /// @nodoc
  /// Make dispatchSemanticsAction hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void dispatchSemanticsAction(int id, int action) =>
      _dispatchSemanticsAction(id, action);

  /// @nodoc
  /// Make beginFrame hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void beginFrame(int microseconds) => _beginFrame(microseconds);

  /// @nodoc
  /// Make drawFrame hook visible for testing.
  /// Do not use for non-testing purposes: catastrophic problems may occur.
  static void drawFrame() => _drawFrame();
}
