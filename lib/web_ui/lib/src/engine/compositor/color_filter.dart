part of engine;

class SkColorFilter {
  js.JsObject skColorFilter;

  SkColorFilter.mode(EngineColorFilter filter) {
    skColorFilter =
        canvasKit['SkColorFilter'].callMethod('MakeBlend', <dynamic>[
      filter._color.value,
      makeSkBlendMode(filter._blendMode),
    ]);
  }

  SkColorFilter.matrix(EngineColorFilter filter) {
    final js.JsArray colorMatrix = js.JsArray();
    colorMatrix.length = 20;
    for (int i = 0; i < 20; i++) {
      colorMatrix[i] = filter._matrix[i];
    }
    skColorFilter = canvasKit['SkColorFilter']
        .callMethod('MakeMatrix', <js.JsArray>[colorMatrix]);
  }

  SkColorFilter.linearToSrgbGamma(EngineColorFilter filter) {
    skColorFilter = canvasKit['SkColorFilter'].callMethod('MakeLinearToSRGBGamma');
  }

  SkColorFilter.srgbToLinearGamma(EngineColorFilter filter) {
    skColorFilter = canvasKit['SkColorFilter'].callMethod('MakeSRGBToLinearGamma');
  }
}
