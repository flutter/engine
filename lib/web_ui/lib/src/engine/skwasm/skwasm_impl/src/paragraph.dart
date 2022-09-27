import 'package:ui/ui.dart' as ui;

class SkwasmLineMetrics implements ui.LineMetrics {
  factory SkwasmLineMetrics({
    required bool hardBreak,
    required double ascent,
    required double descent,
    required double unscaledAscent,
    required double height,
    required double width,
    required double left,
    required double baseline,
    required int lineNumber,
  }) {
    throw UnimplementedError();
  }

  @override
  bool get hardBreak {
    throw UnimplementedError();
  }

  @override
  double get ascent {
    throw UnimplementedError();
  }

  @override
  double get descent {
    throw UnimplementedError();
  }

  @override
  double get unscaledAscent {
    throw UnimplementedError();
  }

  @override
  double get height {
    throw UnimplementedError();
  }

  @override
  double get width {
    throw UnimplementedError();
  }

  @override
  double get left {
    throw UnimplementedError();
  }

  @override
  double get baseline {
    throw UnimplementedError();
  }

  @override
  int get lineNumber {
    throw UnimplementedError();
  }
}

class SkwasmParagraph implements ui.Paragraph {
  double get width {
    throw UnimplementedError();
  }

  double get height {
    throw UnimplementedError();
  }

  double get longestLine {
    throw UnimplementedError();
  }

  double get minIntrinsicWidth {
    throw UnimplementedError();
  }

  double get maxIntrinsicWidth {
    throw UnimplementedError();
  }

  double get alphabeticBaseline {
    throw UnimplementedError();
  }

  double get ideographicBaseline {
    throw UnimplementedError();
  }

  bool get didExceedMaxLines {
    throw UnimplementedError();
  }

  void layout(ui.ParagraphConstraints constraints) {
    throw UnimplementedError();
  }

  List<ui.TextBox> getBoxesForRange(int start, int end,
      {ui.BoxHeightStyle boxHeightStyle = ui.BoxHeightStyle.tight,
      ui.BoxWidthStyle boxWidthStyle = ui.BoxWidthStyle.tight}) {
    throw UnimplementedError();
  }

  ui.TextPosition getPositionForOffset(ui.Offset offset) {
    throw UnimplementedError();
  }

  ui.TextRange getWordBoundary(ui.TextPosition position) {
    throw UnimplementedError();
  }

  ui.TextRange getLineBoundary(ui.TextPosition position) {
    throw UnimplementedError();
  }

  List<ui.TextBox> getBoxesForPlaceholders() {
    throw UnimplementedError();
  }

  List<SkwasmLineMetrics> computeLineMetrics() {
    throw UnimplementedError();
  }
}
