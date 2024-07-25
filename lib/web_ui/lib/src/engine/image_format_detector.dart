// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

/// Detects the image file format and returns the corresponding "Content-Type"
/// value (a.k.a. MIME type).
///
/// The returned value can be passed to `ImageDecoder` when decoding an image.
///
/// Returns null if [data] cannot be mapped to a known content type.
String? detectContentType(Uint8List data) {
  if (debugContentTypeDetector != null) {
    return debugContentTypeDetector!.call(data);
  }

  formatLoop:
  for (final ImageFileFormat format in ImageFileFormat.values) {
    if (data.length < format.header.length) {
      continue;
    }

    for (int i = 0; i < format.header.length; i++) {
      final int? magicByte = format.header[i];
      if (magicByte == null) {
        // Wildcard, accepts everything.
        continue;
      }

      final int headerByte = data[i];
      if (headerByte != magicByte) {
        continue formatLoop;
      }
    }

    return format.contentType;
  }

  if (isAvif(data)) {
    return 'image/avif';
  }

  return null;
}

/// Represents an image file format, such as PNG or JPEG.
class ImageFileFormat {
  const ImageFileFormat(this.header, this.contentType);

  /// First few bytes in the file that uniquely identify the image file format.
  ///
  /// Null elements are treated as wildcard values and are not checked. This is
  /// used to detect formats whose header is split up into multiple disjoint
  /// parts, such that the first part is not unique enough to identify the
  /// format. For example, without this, WebP may be confused with .ani
  /// (animated cursor), .cda, and other formats that start with "RIFF".
  final List<int?> header;

  /// The value that's passed as [_ImageDecoderOptions.type].
  ///
  /// The server typically also uses this value as the "Content-Type" header,
  /// but servers are not required to correctly detect the type. This value
  /// is also known as MIME type.
  final String contentType;

  /// All image file formats known to the Flutter Web engine.
  ///
  /// This list may need to be changed as browsers adopt new formats, and drop
  /// support for obsolete ones.
  ///
  /// This list is checked linearly from top to bottom when detecting an image
  /// type. It should therefore contain the most popular file formats at the
  /// top, and less popular towards the bottom.
  static const List<ImageFileFormat> values = <ImageFileFormat>[
    // ICO is not supported in Chrome. It is deemed too simple and too specific. See also:
    //   https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/modules/webcodecs/image_decoder_external.cc;l=38;drc=fd8802b593110ea18a97ef044f8a40dd24a622ec

    // PNG
    ImageFileFormat(
        <int?>[0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A], 'image/png'),

    // GIF87a
    ImageFileFormat(<int?>[0x47, 0x49, 0x46, 0x38, 0x37, 0x61], 'image/gif'),

    // GIF89a
    ImageFileFormat(<int?>[0x47, 0x49, 0x46, 0x38, 0x39, 0x61], 'image/gif'),

    // JPEG
    ImageFileFormat(<int?>[0xFF, 0xD8, 0xFF], 'image/jpeg'),

    // WebP
    ImageFileFormat(<int?>[
      0x52,
      0x49,
      0x46,
      0x46,
      null,
      null,
      null,
      null,
      0x57,
      0x45,
      0x42,
      0x50
    ], 'image/webp'),

    // BMP
    ImageFileFormat(<int?>[0x42, 0x4D], 'image/bmp'),
  ];
}

/// Function signature of [debugContentTypeDetector], which is the same as the
/// signature of [detectContentType].
typedef DebugContentTypeDetector = String? Function(Uint8List);

/// If not null, replaces the functionality of [detectContentType] with its own.
///
/// This is useful in tests, for example, to test unsupported content types.
DebugContentTypeDetector? debugContentTypeDetector;

/// A string of bytes that every AVIF image contains somewhere in its first 16
/// bytes.
///
/// This signature is necessary but not sufficient, which may lead to false
/// positives. For example, the file may be HEIC or a video. This is OK,
/// because in the worst case, the image decoder fails to decode the file.
/// This is something we must anticipate regardless of this detection logic.
/// The codec must already protect itself from downloaded files lying about
/// their contents.
///
/// The alternative would be to implement a more precise detection, which would
/// add complexity and code size. This is how Chromium does it:
///
/// https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/platform/image-decoders/avif/avif_image_decoder.cc;l=504;drc=fd8802b593110ea18a97ef044f8a40dd24a622ec
final List<int> _avifSignature = 'ftyp'.codeUnits;

/// Optimistically detects whether [data] is an AVIF image file.
bool isAvif(Uint8List data) {
  firstByteLoop:
  for (int i = 0; i < 16; i += 1) {
    for (int j = 0; j < _avifSignature.length; j += 1) {
      if (i + j >= data.length) {
        // Reached EOF without finding the signature.
        return false;
      }
      if (data[i + j] != _avifSignature[j]) {
        continue firstByteLoop;
      }
    }
    return true;
  }
  return false;
}
