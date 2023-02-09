import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

/// A surface that can store the contents of a Surface. Used as overlay for
/// platform views.
class SnapshotSurface {
  /// The root HTML element for this surface.
  final DomElement htmlElement = createDomElement('flt-canvas-container');

  /// The underlying `<canvas>` element used for this snapshot surface.
  DomCanvasElement? htmlCanvas;

  /// The 2D rendering context of [htmlCanvas].
  DomCanvasRenderingContext2D? canvasContext;

  late ui.Size _currentPhysicalSize;

  ui.Size get physicalSize => _currentPhysicalSize;

  /// Creates a <canvas> and context for the given [size].
  void createOrUpdateSurface(ui.Size size) {
    if (size.isEmpty) {
      throw CanvasKitError('Cannot create snapshot surfaces of empty size.');
    }

    _currentPhysicalSize = size;

    if (htmlCanvas == null) {
      _createNewCanvas();
    } else {
      htmlCanvas?.width = size.width.ceilToDouble();
      htmlCanvas?.height = size.height.ceilToDouble();
      _updateLogicalHtmlCanvasSize();
    }
  }

  /// This function is expensive.
  ///
  /// It's better to reuse canvas if possible.
  void _createNewCanvas() {
    // Clear the container, if it's not empty. We're going to create a new <canvas>.
    if (this.htmlCanvas != null) {
      this.htmlCanvas!.remove();
    }

    // If `physicalSize` is not precise, use a slightly bigger canvas. This way
    // we ensure that the rendred picture covers the entire browser window.
    final DomCanvasElement htmlCanvas = createDomCanvasElement(
      width: _currentPhysicalSize.width.ceil(),
      height: _currentPhysicalSize.height.ceil(),
    );

    this.htmlCanvas = htmlCanvas;

    // The DOM elements used to render pictures are used purely to put pixels on
    // the screen. They have no semantic information. If an assistive technology
    // attempts to scan picture content it will look like garbage and confuse
    // users. UI semantics are exported as a separate DOM tree rendered parallel
    // to pictures.
    //
    // Why are layer and scene elements not hidden from ARIA? Because those
    // elements may contain platform views, and platform views must be
    // accessible.
    htmlCanvas.setAttribute('aria-hidden', 'true');

    htmlCanvas.style.position = 'absolute';
    _updateLogicalHtmlCanvasSize();

    canvasContext = htmlCanvas.context2D;
    canvasContext?.globalCompositeOperation = 'copy';

    htmlElement.append(htmlCanvas);
  }

  /// Sets the CSS size of the canvas so that canvas pixels are 1:1 with device
  /// pixels.
  ///
  /// The logical size of the canvas is not based on the size of the window
  /// but on the size of the canvas, which, due to `ceil()` above, may not be
  /// the same as the window. We do not round/floor/ceil the logical size as
  /// CSS pixels can contain more than one physical pixel and therefore to
  /// match the size of the window precisely we use the most precise floating
  /// point value we can get.
  void _updateLogicalHtmlCanvasSize() {
    final ui.Size logicalSize = _currentPhysicalSize / window.devicePixelRatio;
    final DomCSSStyleDeclaration style = htmlCanvas!.style;
    style.width = '${logicalSize.width}px';
    style.height = '${logicalSize.height}px';
  }

  void dispose() {
    htmlElement.remove();
  }
}
