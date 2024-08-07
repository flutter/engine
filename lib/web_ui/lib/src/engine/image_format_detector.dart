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
ImageType? detectImageType(Uint8List data) {
  if (debugImageTypeDetector != null) {
    return debugImageTypeDetector!.call(data);
  }

  formatLoop:
  for (final ImageFileSignature format in ImageFileSignature.values) {
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

    // If we detect a WebP image, check if it is animated.
    if (format.imageType == ImageType.webp) {
      final WebpHeaderReader webpHeaderReader =
          WebpHeaderReader(data.buffer.asByteData());
      if (webpHeaderReader.isAnimated()) {
        return ImageType.animatedWebp;
      }
    }
    return format.imageType;
  }

  if (isAvif(data)) {
    return ImageType.avif;
  }

  return null;
}

/// The supported image file formats in Flutter Web.
enum ImageFileType {
  png,
  gif,
  jpeg,
  webp,
  bmp,
  avif,
}

/// The file format of the image, and whether or not it is animated.
enum ImageType {
  // TODO(harryterkelsen): If the image is animated, we use Skia to decode.
  // This is currently too conservative, assuming all GIF and WEBP images are
  // animated. We should detect if they are actually animated by reading the
  // image headers, https://github.com/flutter/flutter/issues/151911.
  png(ImageFileType.png, isAnimated: false),
  gif(ImageFileType.gif, isAnimated: false),
  animatedGif(ImageFileType.gif, isAnimated: true),
  jpeg(ImageFileType.jpeg, isAnimated: false),
  webp(ImageFileType.webp, isAnimated: false),
  animatedWebp(ImageFileType.webp, isAnimated: true),
  bmp(ImageFileType.bmp, isAnimated: false),
  avif(ImageFileType.avif, isAnimated: false);

  const ImageType(this.fileType, {required this.isAnimated});

  final ImageFileType fileType;
  final bool isAnimated;

  /// The MIME type for this image (e.g 'image/jpeg').
  String get mimeType => 'image/${fileType.name}';
}

/// The signature bytes in an image file that identify the format.
enum ImageFileSignature {
  png(
    <int?>[0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A],
    ImageType.png,
  ),
  gif87a(
    <int?>[0x47, 0x49, 0x46, 0x38, 0x37, 0x61],
    ImageType.animatedGif,
  ),
  gif89a(
    <int?>[0x47, 0x49, 0x46, 0x38, 0x39, 0x61],
    ImageType.animatedGif,
  ),
  jpeg(
    <int?>[0xFF, 0xD8, 0xFF],
    ImageType.jpeg,
  ),
  webp(
    <int?>[
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
    ],
    ImageType.webp,
  ),
  bmp(
    <int?>[0x42, 0x4D],
    ImageType.bmp,
  );

  const ImageFileSignature(this.header, this.imageType);

  /// First few bytes in the file that uniquely identify the image file format.
  ///
  /// Null elements are treated as wildcard values and are not checked. This is
  /// used to detect formats whose header is split up into multiple disjoint
  /// parts, such that the first part is not unique enough to identify the
  /// format. For example, without this, WebP may be confused with .ani
  /// (animated cursor), .cda, and other formats that start with "RIFF".
  final List<int?> header;

  /// The type of image that has the signature bytes in [header] in its header.
  final ImageType imageType;
}

/// Function signature of [debugImageTypeDetector], which is the same as the
/// signature of [detectImageType].
typedef DebugImageTypeDetector = ImageType? Function(Uint8List);

/// If not null, replaces the functionality of [detectImageType] with its own.
///
/// This is useful in tests, for example, to test unsupported content types.
DebugImageTypeDetector? debugImageTypeDetector;

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

/// Reads the header of a WebP file to determine if it is animated or not.
///
/// See https://developers.google.com/speed/webp/docs/riff_container
class WebpHeaderReader {
  WebpHeaderReader(this.bytes);

  final ByteData bytes;

  /// The current position we are reading from in bytes.
  int _position = 0;

  /// Returns [true] if this WebP is animated.
  bool isAnimated() {
    final bool isWebP = _readWebpHeader();
    if (!isWebP) {
      return false;
    }
    // If this is an animated WebP, then it must have a 'VP8X' chunk header
    // with the 'animation' bit set. See: https://developers.google.com/speed/webp/docs/riff_container#extended_file_format
    final bool isVP8X = _readChunkHeader('VP8X');
    if (!isVP8X) {
      return false;
    }

    // If this is a VP8X style WebP, then the next byte will have a bit which
    // indicates whether it is animated or not.
    final int headerByte = _readUint8();
    const int animatedMask = 1 << 1;
    return headerByte & animatedMask != 0;
  }

  /// Reads a RIFF chunk header. Returns [false] if the header FourCC isn't
  /// [expectedHeader].
  bool _readChunkHeader(String expectedHeader) {
    final String chunkFourCC = _readFourCC();
    // Read chunk size.
    _readUint32();
    return chunkFourCC == expectedHeader;
  }

  /// Reads the WebP header. Returns [false] if this is not a valid WebP header.
  bool _readWebpHeader() {
    final String riffBytes = _readFourCC();

    // Read file size byte.
    _readUint32();

    final String webpBytes = _readFourCC();
    return riffBytes == 'RIFF' && webpBytes == 'WEBP';
  }

  int _readUint32() {
    final int result = bytes.getUint32(_position, Endian.little);
    _position += 4;
    return result;
  }

  int _readUint8() {
    final int result = bytes.getUint8(_position);
    _position += 1;
    return result;
  }

  /// A four-character code is a uint32 created by concatenating four ASCII
  /// characters in little-endian order.
  String _readFourCC() {
    final List<int> chars = <int>[
      bytes.getUint8(_position),
      bytes.getUint8(_position + 1),
      bytes.getUint8(_position + 2),
      bytes.getUint8(_position + 3),
    ];
    _position += 4;
    return String.fromCharCodes(chars);
  }
}
