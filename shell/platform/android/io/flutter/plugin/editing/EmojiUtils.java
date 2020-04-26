// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.editing;

/** Emoji data based on http://www.unicode.org/reports/tr51/#emoji_data */
public class EmojiUtils {

  public static final int COMBINING_ENCLOSING_KEYCAP = 0x20E3;
  public static final int CANCEL_TAG = 0xE007F;
  public static final int ZERO_WIDTH_JOINER = 0x200D;

  public static boolean isEmoji(int c) {
    if (c < 0x0023 || c > 0x1FFFD) {
      return false;
    }
    return c == 0x0023
        || c == 0x002A
        || c == 0x00A9
        || c == 0x00AE
        || c == 0x203C
        || c == 0x2049
        || c == 0x2122
        || c == 0x2139
        || c == 0x2328
        || c == 0x23CF
        || c == 0x24C2
        || c == 0x25B6
        || c == 0x25C0
        || c == 0x2714
        || c == 0x2716
        || c == 0x271D
        || c == 0x2721
        || c == 0x2728
        || c == 0x2744
        || c == 0x2747
        || c == 0x274C
        || c == 0x274E
        || c == 0x2757
        || c == 0x27A1
        || c == 0x27B0
        || c == 0x27BF
        || c == 0x2B50
        || c == 0x2B55
        || c == 0x3030
        || c == 0x303D
        || c == 0x3297
        || c == 0x3299
        || c == 0x1F18E
        || c == 0x1F21A
        || c == 0x1F22F
        || c == 0x200D
        || c == 0x20E3
        || c == 0x2388
        || c == 0x1F12F
        || (0x0030 <= c && c <= 0x0039)
        || (0x2194 <= c && c <= 0x2199)
        || (0x21A9 <= c && c <= 0x21AA)
        || (0x231A <= c && c <= 0x231B)
        || (0x23E9 <= c && c <= 0x23F3)
        || (0x23F8 <= c && c <= 0x23FA)
        || (0x25AA <= c && c <= 0x25AB)
        || (0x25FB <= c && c <= 0x25FE)
        || (0x2600 <= c && c <= 0x2605)
        || (0x2607 <= c && c <= 0x2612)
        || (0x2614 <= c && c <= 0x2685)
        || (0x2690 <= c && c <= 0x2705)
        || (0x2708 <= c && c <= 0x2712)
        || (0x2733 <= c && c <= 0x2734)
        || (0x2753 <= c && c <= 0x2755)
        || (0x2763 <= c && c <= 0x2767)
        || (0x2795 <= c && c <= 0x2797)
        || (0x2934 <= c && c <= 0x2935)
        || (0x2B05 <= c && c <= 0x2B07)
        || (0x2B1B <= c && c <= 0x2B1C)
        || (0x1F000 <= c && c <= 0x1F0FF)
        || (0x1F16C <= c && c <= 0x1F171)
        || (0x1F17E <= c && c <= 0x1F17F)
        || (0x1F191 <= c && c <= 0x1F19A)
        || (0x1F1AD <= c && c <= 0x1F1FF)
        || (0x1F201 <= c && c <= 0x1F20F)
        || (0x1F232 <= c && c <= 0x1F23A)
        || (0x1F249 <= c && c <= 0x1F53D)
        || (0x1F546 <= c && c <= 0x1F64F)
        || (0x1F680 <= c && c <= 0x1F6FF)
        || (0x1F7D5 <= c && c <= 0x1F7FF)
        || (0x1F90C <= c && c <= 0x1F93A)
        || (0x1F93C <= c && c <= 0x1F945)
        || (0x1F947 <= c && c <= 0x1FAFF)
        || (0x1F10D <= c && c <= 0x1F10F)
        || (0x1F23C <= c && c <= 0x1F23F)
        || (0x1F774 <= c && c <= 0x1F77F)
        || (0x1F80C <= c && c <= 0x1F80F)
        || (0x1F848 <= c && c <= 0x1F84F)
        || (0x1F85A <= c && c <= 0x1F85F)
        || (0x1F888 <= c && c <= 0x1F88F)
        || (0x1F8AE <= c && c <= 0x1F8FF)
        || (0x1FC00 <= c && c <= 0x1FFFD);
  }

  public static boolean isRegionalIndicatorSymbol(int codePoint) {
    return 0x1F1E6 <= codePoint && codePoint <= 0x1F1FF;
  }

  public static boolean isTagSpecChar(int codePoint) {
    return 0xE0020 <= codePoint && codePoint <= 0xE007E;
  }

  public static boolean isKeycapBase(int codePoint) {
    return ('0' <= codePoint && codePoint <= '9') || codePoint == '#' || codePoint == '*';
  }

  public static boolean isEmojiModifier(int codePoint) {
    return 0x1F3FB <= codePoint && codePoint <= 0x1F3FF;
  }

  public static boolean isEmojiModifierBase(int c) {
    if (c < 0x261D || c > 0x1F9DD) {
      return false;
    }
    return c == 0x261D
        || c == 0x26F9
        || c == 0x1F385
        || c == 0x1F3C7
        || c == 0x1F47C
        || c == 0x1F48F
        || c == 0x1F491
        || c == 0x1F4AA
        || c == 0x1F57A
        || c == 0x1F590
        || c == 0x1F6A3
        || c == 0x1F6C0
        || c == 0x1F6CC
        || c == 0x1F90C
        || c == 0x1F90F
        || c == 0x1F926
        || c == 0x1F977
        || c == 0x1F9BB
        || (0x270A <= c && c <= 0x270D)
        || (0x1F3C2 <= c && c <= 0x1F3C4)
        || (0x1F3CA <= c && c <= 0x1F3CC)
        || (0x1F442 <= c && c <= 0x1F443)
        || (0x1F446 <= c && c <= 0x1F450)
        || (0x1F466 <= c && c <= 0x1F478)
        || (0x1F481 <= c && c <= 0x1F483)
        || (0x1F485 <= c && c <= 0x1F487)
        || (0x1F574 <= c && c <= 0x1F575)
        || (0x1F595 <= c && c <= 0x1F596)
        || (0x1F645 <= c && c <= 0x1F647)
        || (0x1F64B <= c && c <= 0x1F64F)
        || (0x1F6B4 <= c && c <= 0x1F6B6)
        || (0x1F918 <= c && c <= 0x1F91F)
        || (0x1F930 <= c && c <= 0x1F939)
        || (0x1F93C <= c && c <= 0x1F93E)
        || (0x1F9B5 <= c && c <= 0x1F9B6)
        || (0x1F9B8 <= c && c <= 0x1F9B9)
        || (0x1F9CD <= c && c <= 0x1F9CF)
        || (0x1F9D1 <= c && c <= 0x1F9DD);
  }

  public static boolean isVariationSelector(int codePoint) {
    return 0xFE0E <= codePoint && codePoint <= 0xFE0F;
  }

  public static boolean isVariationBase(int c) {
    if (c < 0x0023 || c > 0x1F6F3) {
      return false;
    }
    return c == 0x0023
        || c == 0x002A
        || c == 0x00A9
        || c == 0x00AE
        || c == 0x203C
        || c == 0x2049
        || c == 0x2122
        || c == 0x2139
        || c == 0x2328
        || c == 0x23CF
        || c == 0x24C2
        || c == 0x25B6
        || c == 0x25C0
        || c == 0x260E
        || c == 0x2611
        || c == 0x2618
        || c == 0x261D
        || c == 0x2620
        || c == 0x2626
        || c == 0x262A
        || c == 0x2640
        || c == 0x2642
        || c == 0x2663
        || c == 0x2668
        || c == 0x267B
        || c == 0x2699
        || c == 0x26A7
        || c == 0x26C8
        || c == 0x26CF
        || c == 0x26D1
        || c == 0x26FD
        || c == 0x2702
        || c == 0x270F
        || c == 0x2712
        || c == 0x2714
        || c == 0x2716
        || c == 0x271D
        || c == 0x2721
        || c == 0x2744
        || c == 0x2747
        || c == 0x2753
        || c == 0x2757
        || c == 0x27A1
        || c == 0x2B50
        || c == 0x2B55
        || c == 0x3030
        || c == 0x303D
        || c == 0x3297
        || c == 0x3299
        || c == 0x1F004
        || c == 0x1F202
        || c == 0x1F21A
        || c == 0x1F22F
        || c == 0x1F237
        || c == 0x1F315
        || c == 0x1F31C
        || c == 0x1F321
        || c == 0x1F336
        || c == 0x1F378
        || c == 0x1F37D
        || c == 0x1F393
        || c == 0x1F3A7
        || c == 0x1F3C2
        || c == 0x1F3C4
        || c == 0x1F3C6
        || c == 0x1F3ED
        || c == 0x1F3F3
        || c == 0x1F3F5
        || c == 0x1F3F7
        || c == 0x1F408
        || c == 0x1F415
        || c == 0x1F41F
        || c == 0x1F426
        || c == 0x1F43F
        || c == 0x1F453
        || c == 0x1F46A
        || c == 0x1F47D
        || c == 0x1F4A3
        || c == 0x1F4B0
        || c == 0x1F4B3
        || c == 0x1F4BB
        || c == 0x1F4BF
        || c == 0x1F4CB
        || c == 0x1F4DA
        || c == 0x1F4DF
        || c == 0x1F4F7
        || c == 0x1F4FD
        || c == 0x1F508
        || c == 0x1F50D
        || c == 0x1F587
        || c == 0x1F590
        || c == 0x1F5A5
        || c == 0x1F5A8
        || c == 0x1F5BC
        || c == 0x1F5E1
        || c == 0x1F5E3
        || c == 0x1F5E8
        || c == 0x1F5EF
        || c == 0x1F5F3
        || c == 0x1F5FA
        || c == 0x1F610
        || c == 0x1F687
        || c == 0x1F68D
        || c == 0x1F691
        || c == 0x1F694
        || c == 0x1F698
        || c == 0x1F6AD
        || c == 0x1F6B2
        || c == 0x1F6BC
        || c == 0x1F6CB
        || c == 0x1F6E9
        || c == 0x1F6F0
        || c == 0x1F6F3
        || (0x0030 <= c && c <= 0x0039)
        || (0x2194 <= c && c <= 0x2199)
        || (0x21A9 <= c && c <= 0x21AA)
        || (0x231A <= c && c <= 0x231B)
        || (0x23E9 <= c && c <= 0x23EA)
        || (0x23ED <= c && c <= 0x23EF)
        || (0x23F1 <= c && c <= 0x23F3)
        || (0x23F8 <= c && c <= 0x23FA)
        || (0x25AA <= c && c <= 0x25AB)
        || (0x25FB <= c && c <= 0x25FE)
        || (0x2600 <= c && c <= 0x2604)
        || (0x2614 <= c && c <= 0x2615)
        || (0x2622 <= c && c <= 0x2623)
        || (0x262E <= c && c <= 0x262F)
        || (0x2638 <= c && c <= 0x263A)
        || (0x2648 <= c && c <= 0x2653)
        || (0x265F <= c && c <= 0x2660)
        || (0x2665 <= c && c <= 0x2666)
        || (0x267E <= c && c <= 0x267F)
        || (0x2692 <= c && c <= 0x2697)
        || (0x269B <= c && c <= 0x269C)
        || (0x26A0 <= c && c <= 0x26A1)
        || (0x26AA <= c && c <= 0x26AB)
        || (0x26B0 <= c && c <= 0x26B1)
        || (0x26BD <= c && c <= 0x26BE)
        || (0x26C4 <= c && c <= 0x26C5)
        || (0x26D3 <= c && c <= 0x26D4)
        || (0x26E9 <= c && c <= 0x26EA)
        || (0x26F0 <= c && c <= 0x26F5)
        || (0x26F7 <= c && c <= 0x26FA)
        || (0x2708 <= c && c <= 0x2709)
        || (0x270C <= c && c <= 0x270D)
        || (0x2733 <= c && c <= 0x2734)
        || (0x2763 <= c && c <= 0x2764)
        || (0x2934 <= c && c <= 0x2935)
        || (0x2B05 <= c && c <= 0x2B07)
        || (0x2B1B <= c && c <= 0x2B1C)
        || (0x1F170 <= c && c <= 0x1F171)
        || (0x1F17E <= c && c <= 0x1F17F)
        || (0x1F30D <= c && c <= 0x1F30F)
        || (0x1F324 <= c && c <= 0x1F32C)
        || (0x1F396 <= c && c <= 0x1F397)
        || (0x1F399 <= c && c <= 0x1F39B)
        || (0x1F39E <= c && c <= 0x1F39F)
        || (0x1F3AC <= c && c <= 0x1F3AE)
        || (0x1F3CA <= c && c <= 0x1F3CE)
        || (0x1F3D4 <= c && c <= 0x1F3E0)
        || (0x1F441 <= c && c <= 0x1F442)
        || (0x1F446 <= c && c <= 0x1F449)
        || (0x1F44D <= c && c <= 0x1F44E)
        || (0x1F4E4 <= c && c <= 0x1F4E6)
        || (0x1F4EA <= c && c <= 0x1F4ED)
        || (0x1F4F9 <= c && c <= 0x1F4FB)
        || (0x1F512 <= c && c <= 0x1F513)
        || (0x1F549 <= c && c <= 0x1F54A)
        || (0x1F550 <= c && c <= 0x1F567)
        || (0x1F56F <= c && c <= 0x1F570)
        || (0x1F573 <= c && c <= 0x1F579)
        || (0x1F58A <= c && c <= 0x1F58D)
        || (0x1F5B1 <= c && c <= 0x1F5B2)
        || (0x1F5C2 <= c && c <= 0x1F5C4)
        || (0x1F5D1 <= c && c <= 0x1F5D3)
        || (0x1F5DC <= c && c <= 0x1F5DE)
        || (0x1F6B9 <= c && c <= 0x1F6BA)
        || (0x1F6CD <= c && c <= 0x1F6CF)
        || (0x1F6E0 <= c && c <= 0x1F6E5);
  }
}
