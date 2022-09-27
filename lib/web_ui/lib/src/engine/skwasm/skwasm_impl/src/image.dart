import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

class SkwasmImage implements ui.Image {
  @override
  int get width {
    throw UnimplementedError();
  }

  @override
  int get height {
    throw UnimplementedError();
  }

  @override
  Future<ByteData?> toByteData(
      {ui.ImageByteFormat format = ui.ImageByteFormat.rawRgba}) {
    throw UnimplementedError();
  }

  @override
  void dispose() {
    throw UnimplementedError();
  }

  @override
  bool get debugDisposed {
    throw UnimplementedError();
  }

  @override
  SkwasmImage clone() => this;

  @override
  bool isCloneOf(ui.Image other) => other == this;

  @override
  List<StackTrace>? debugGetOpenHandleStackTraces() => null;

  @override
  String toString() => '[$width\u00D7$height]';
}
