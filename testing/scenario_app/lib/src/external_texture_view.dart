import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';
import 'dart:ui';
import 'scenario.dart';


class ExternalTextureScenario extends Scenario {
  /// Creates the PlatformView scenario.
  ///
  /// The [window] parameter must not be null.
  ExternalTextureScenario(Window window)
      : assert(window != null),
        super(window) {
    final ByteData data = ByteData(1);
    data.setUint8(0, 1);
    window.sendPlatformMessage(
      'create_external_texture',
      data,
      null,
    );
  }

  int _textureId;

  @override
  void onBeginFrame(Duration duration) {

    print('begin frame 0000');

    final SceneBuilder builder = SceneBuilder();

    builder.pushOffset(0, 0);

    if (_textureId != null) {
      print('begin frame 1111');

      builder.addTexture(_textureId, offset: const Offset(0, 0), width: 480, height: 480);
    }


    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }

  @override
  void onUpdateData(ByteData data) {
    super.onUpdateData(data);
    if (data != null) {
      String string = utf8.decode(data.buffer.asUint8List());
      _textureId = int.parse(string);
      print('update textureid $_textureId');

      final SceneBuilder builder = SceneBuilder();

      builder.pushOffset(0, 0);

      if (_textureId != null) {
        print('begin frame 1111');

        builder.addTexture(_textureId, offset: const Offset(0, 0), width: 480, height: 480);
      }


      final Scene scene = builder.build();
      window.render(scene);
      scene.dispose();
    }
  }
}
