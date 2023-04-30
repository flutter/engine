// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

const String flutterLogoPngAsDataUri = 'data:image/png;base64,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';

// Following [_kindToCssValueMap] array definition is from flutter engine /lib/web_ui/lib/src/engine/mouse_cursor.dart
//   
// Map from Flutter's kind values to CSS's cursor values.
//
// This map must be kept in sync with Flutter framework's
// rendering/mouse_cursor.dart.
const Map<String, String> _kindToCssValueMap = <String, String>{
  'alias': 'alias',
  'allScroll': 'all-scroll',
  'basic': 'default',
  'cell': 'cell',
  'click': 'pointer',
  'contextMenu': 'context-menu',
  'copy': 'copy',
  'forbidden': 'not-allowed',
  'grab': 'grab',
  'grabbing': 'grabbing',
  'help': 'help',
  'move': 'move',
  'none': 'none',
  'noDrop': 'no-drop',
  'precise': 'crosshair',
  'progress': 'progress',
  'text': 'text',
  'resizeColumn': 'col-resize',
  'resizeDown': 's-resize',
  'resizeDownLeft': 'sw-resize',
  'resizeDownRight': 'se-resize',
  'resizeLeft': 'w-resize',
  'resizeLeftRight': 'ew-resize',
  'resizeRight': 'e-resize',
  'resizeRow': 'row-resize',
  'resizeUp': 'n-resize',
  'resizeUpDown': 'ns-resize',
  'resizeUpLeft': 'nw-resize',
  'resizeUpRight': 'ne-resize',
  'resizeUpLeftDownRight': 'nwse-resize',
  'resizeUpRightDownLeft': 'nesw-resize',
  'verticalText': 'vertical-text',
  'wait': 'wait',
  'zoomIn': 'zoom-in',
  'zoomOut': 'zoom-out',
};

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  ensureFlutterViewEmbedderInitialized();
  final DomElement flutterViewElement = flutterViewEmbedder.flutterViewElement;

  setUp(() {
  });

  MouseCursor.initialize();
  final MouseCursor? mouseCursorTry = MouseCursor.instance;

  assert(mouseCursorTry!=null,'Error MouseCursor.instance is null');

  final MouseCursor mouseCursorInstance = mouseCursorTry!;

  void setCursorKind(String kindToTest) {
    mouseCursorInstance.activateSystemCursor( kindToTest );
  }

  String getSetCursorCSSStyle() {
    final DomCSSStyleDeclaration style = domWindow.getComputedStyle(flutterViewElement);
    return style.getPropertyValue('cursor');
  }
  
  // Make sure that all 'built in' MouseCursor kinds ([_kindToCssValueMap]) make it through to there css style mapping
  test('MouseCursor.activateSystemCursor test built in system cursor to browser css map', () {
    _kindToCssValueMap.forEach( (String key, String expectedResult) {
        setCursorKind(key);
        expect( getSetCursorCSSStyle(), equals(expectedResult) );
      }
    );
  });

  // We need to test that cursor key/kind values that start with 
  // 'url(..)',  'image-set(...)'/'-webkit-image-set(...)' css commands make it through.
  test('MouseCursor.activateSystemCursor allows url/-webkit-image-set kind value prefix to pass', () {
    const String urlCss = 'url("cursor.png")';
    const String webkitImageSetCss = '-webkit-image-set($urlCss 1x) 0 0, $urlCss, help';

    setCursorKind('grabbing'); // reset to different cursor before testing 'url...'

    setCursorKind('$urlCss 4 12, default');
    expect( getSetCursorCSSStyle(), startsWith('url') );

    setCursorKind('progress'); // reset to different cursor before testing '-webkit-image-set...'

    setCursorKind(webkitImageSetCss);
    expect( getSetCursorCSSStyle(), startsWith('-webkit-image-set') );
    
    // we do not test the 'webkit-image-set' prefix because browsers currently throw that out.
  });

  test('MouseCursor.activateSystemCursor allows url/-webkit-image-set (with datauri url) kind value prefix to pass', () {
    const String urlCss = 'url("$flutterLogoPngAsDataUri")';
    const String webkitImageSetCss = '-webkit-image-set($urlCss 1x) 0 0, $urlCss, help';

    setCursorKind('move'); // reset to different cursor before testing 'url...'

    setCursorKind('$urlCss 4 12, default');
    expect( getSetCursorCSSStyle(), startsWith('url') );

    setCursorKind('progress'); // reset to different cursor before testing '-webkit-image-set...'

    setCursorKind(webkitImageSetCss);
    expect( getSetCursorCSSStyle(), startsWith('-webkit-image-set') );

    // we do not test the 'webkit-image-set' prefix because browsers currently throw that out.
  });
}
