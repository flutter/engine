// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "key_mapping.h"

#include <glib.h>
#include <map>

#include "flutter/shell/platform/linux/fl_key_embedder_responder_private.h"

// DO NOT EDIT -- DO NOT EDIT -- DO NOT EDIT
// This file is generated by
// flutter/flutter@dev/tools/gen_keycodes/bin/gen_keycodes.dart and should not
// be edited directly.
//
// Edit the template dev/tools/gen_keycodes/data/gtk_key_mapping_cc.tmpl
// instead. See dev/tools/gen_keycodes/README.md for more information.

std::map<uint64_t, uint64_t> xkb_to_physical_key_map = {
    {0x00000009, 0x00070029},  // escape
    {0x0000000a, 0x0007001e},  // digit1
    {0x0000000b, 0x0007001f},  // digit2
    {0x0000000c, 0x00070020},  // digit3
    {0x0000000d, 0x00070021},  // digit4
    {0x0000000e, 0x00070022},  // digit5
    {0x0000000f, 0x00070023},  // digit6
    {0x00000010, 0x00070024},  // digit7
    {0x00000011, 0x00070025},  // digit8
    {0x00000012, 0x00070026},  // digit9
    {0x00000013, 0x00070027},  // digit0
    {0x00000014, 0x0007002d},  // minus
    {0x00000015, 0x0007002e},  // equal
    {0x00000016, 0x0007002a},  // backspace
    {0x00000017, 0x0007002b},  // tab
    {0x00000018, 0x00070014},  // keyQ
    {0x00000019, 0x0007001a},  // keyW
    {0x0000001a, 0x00070008},  // keyE
    {0x0000001b, 0x00070015},  // keyR
    {0x0000001c, 0x00070017},  // keyT
    {0x0000001d, 0x0007001c},  // keyY
    {0x0000001e, 0x00070018},  // keyU
    {0x0000001f, 0x0007000c},  // keyI
    {0x00000020, 0x00070012},  // keyO
    {0x00000021, 0x00070013},  // keyP
    {0x00000022, 0x0007002f},  // bracketLeft
    {0x00000023, 0x00070030},  // bracketRight
    {0x00000024, 0x00070028},  // enter
    {0x00000025, 0x000700e0},  // controlLeft
    {0x00000026, 0x00070004},  // keyA
    {0x00000027, 0x00070016},  // keyS
    {0x00000028, 0x00070007},  // keyD
    {0x00000029, 0x00070009},  // keyF
    {0x0000002a, 0x0007000a},  // keyG
    {0x0000002b, 0x0007000b},  // keyH
    {0x0000002c, 0x0007000d},  // keyJ
    {0x0000002d, 0x0007000e},  // keyK
    {0x0000002e, 0x0007000f},  // keyL
    {0x0000002f, 0x00070033},  // semicolon
    {0x00000030, 0x00070034},  // quote
    {0x00000031, 0x00070035},  // backquote
    {0x00000032, 0x000700e1},  // shiftLeft
    {0x00000033, 0x00070031},  // backslash
    {0x00000034, 0x0007001d},  // keyZ
    {0x00000035, 0x0007001b},  // keyX
    {0x00000036, 0x00070006},  // keyC
    {0x00000037, 0x00070019},  // keyV
    {0x00000038, 0x00070005},  // keyB
    {0x00000039, 0x00070011},  // keyN
    {0x0000003a, 0x00070010},  // keyM
    {0x0000003b, 0x00070036},  // comma
    {0x0000003c, 0x00070037},  // period
    {0x0000003d, 0x00070038},  // slash
    {0x0000003e, 0x000700e5},  // shiftRight
    {0x0000003f, 0x00070055},  // numpadMultiply
    {0x00000040, 0x000700e2},  // altLeft
    {0x00000041, 0x0007002c},  // space
    {0x00000042, 0x00070039},  // capsLock
    {0x00000043, 0x0007003a},  // f1
    {0x00000044, 0x0007003b},  // f2
    {0x00000045, 0x0007003c},  // f3
    {0x00000046, 0x0007003d},  // f4
    {0x00000047, 0x0007003e},  // f5
    {0x00000048, 0x0007003f},  // f6
    {0x00000049, 0x00070040},  // f7
    {0x0000004a, 0x00070041},  // f8
    {0x0000004b, 0x00070042},  // f9
    {0x0000004c, 0x00070043},  // f10
    {0x0000004d, 0x00070053},  // numLock
    {0x0000004e, 0x00070047},  // scrollLock
    {0x0000004f, 0x0007005f},  // numpad7
    {0x00000050, 0x00070060},  // numpad8
    {0x00000051, 0x00070061},  // numpad9
    {0x00000052, 0x00070056},  // numpadSubtract
    {0x00000053, 0x0007005c},  // numpad4
    {0x00000054, 0x0007005d},  // numpad5
    {0x00000055, 0x0007005e},  // numpad6
    {0x00000056, 0x00070057},  // numpadAdd
    {0x00000057, 0x00070059},  // numpad1
    {0x00000058, 0x0007005a},  // numpad2
    {0x00000059, 0x0007005b},  // numpad3
    {0x0000005a, 0x00070062},  // numpad0
    {0x0000005b, 0x00070063},  // numpadDecimal
    {0x0000005d, 0x00070094},  // lang5
    {0x0000005e, 0x00070064},  // intlBackslash
    {0x0000005f, 0x00070044},  // f11
    {0x00000060, 0x00070045},  // f12
    {0x00000061, 0x00070087},  // intlRo
    {0x00000062, 0x00070092},  // lang3
    {0x00000063, 0x00070093},  // lang4
    {0x00000064, 0x0007008a},  // convert
    {0x00000065, 0x00070088},  // kanaMode
    {0x00000066, 0x0007008b},  // nonConvert
    {0x00000068, 0x00070058},  // numpadEnter
    {0x00000069, 0x000700e4},  // controlRight
    {0x0000006a, 0x00070054},  // numpadDivide
    {0x0000006b, 0x00070046},  // printScreen
    {0x0000006c, 0x000700e6},  // altRight
    {0x0000006e, 0x0007004a},  // home
    {0x0000006f, 0x00070052},  // arrowUp
    {0x00000070, 0x0007004b},  // pageUp
    {0x00000071, 0x00070050},  // arrowLeft
    {0x00000072, 0x0007004f},  // arrowRight
    {0x00000073, 0x0007004d},  // end
    {0x00000074, 0x00070051},  // arrowDown
    {0x00000075, 0x0007004e},  // pageDown
    {0x00000076, 0x00070049},  // insert
    {0x00000077, 0x0007004c},  // delete
    {0x00000079, 0x0007007f},  // audioVolumeMute
    {0x0000007a, 0x00070081},  // audioVolumeDown
    {0x0000007b, 0x00070080},  // audioVolumeUp
    {0x0000007c, 0x00070066},  // power
    {0x0000007d, 0x00070067},  // numpadEqual
    {0x0000007e, 0x000700d7},  // numpadSignChange
    {0x0000007f, 0x00070048},  // pause
    {0x00000080, 0x000c029f},  // showAllWindows
    {0x00000081, 0x00070085},  // numpadComma
    {0x00000082, 0x00070090},  // lang1
    {0x00000083, 0x00070091},  // lang2
    {0x00000084, 0x00070089},  // intlYen
    {0x00000085, 0x000700e3},  // metaLeft
    {0x00000086, 0x000700e7},  // metaRight
    {0x00000087, 0x00070065},  // contextMenu
    {0x00000088, 0x000c0226},  // browserStop
    {0x00000089, 0x00070079},  // again
    {0x0000008b, 0x0007007a},  // undo
    {0x0000008c, 0x00070077},  // select
    {0x0000008d, 0x0007007c},  // copy
    {0x0000008e, 0x00070074},  // open
    {0x0000008f, 0x0007007d},  // paste
    {0x00000090, 0x0007007e},  // find
    {0x00000091, 0x0007007b},  // cut
    {0x00000092, 0x00070075},  // help
    {0x00000094, 0x000c0192},  // launchApp2
    {0x00000096, 0x00010082},  // sleep
    {0x00000097, 0x00010083},  // wakeUp
    {0x00000098, 0x000c0194},  // launchApp1
    {0x0000009e, 0x000c0196},  // launchInternetBrowser
    {0x000000a0, 0x000c019e},  // lockScreen
    {0x000000a3, 0x000c018a},  // launchMail
    {0x000000a4, 0x000c022a},  // browserFavorites
    {0x000000a6, 0x000c0224},  // browserBack
    {0x000000a7, 0x000c0225},  // browserForward
    {0x000000a9, 0x000c00b8},  // eject
    {0x000000ab, 0x000c00b5},  // mediaTrackNext
    {0x000000ac, 0x000c00cd},  // mediaPlayPause
    {0x000000ad, 0x000c00b6},  // mediaTrackPrevious
    {0x000000ae, 0x000c00b7},  // mediaStop
    {0x000000af, 0x000c00b2},  // mediaRecord
    {0x000000b0, 0x000c00b4},  // mediaRewind
    {0x000000b1, 0x000c008c},  // launchPhone
    {0x000000b3, 0x000c0183},  // mediaSelect
    {0x000000b4, 0x000c0223},  // browserHome
    {0x000000b5, 0x000c0227},  // browserRefresh
    {0x000000b6, 0x000c0094},  // exit
    {0x000000bb, 0x000700b6},  // numpadParenLeft
    {0x000000bc, 0x000700b7},  // numpadParenRight
    {0x000000bd, 0x000c0201},  // newKey
    {0x000000be, 0x000c0279},  // redo
    {0x000000bf, 0x00070068},  // f13
    {0x000000c0, 0x00070069},  // f14
    {0x000000c1, 0x0007006a},  // f15
    {0x000000c2, 0x0007006b},  // f16
    {0x000000c3, 0x0007006c},  // f17
    {0x000000c4, 0x0007006d},  // f18
    {0x000000c5, 0x0007006e},  // f19
    {0x000000c6, 0x0007006f},  // f20
    {0x000000c7, 0x00070070},  // f21
    {0x000000c8, 0x00070071},  // f22
    {0x000000c9, 0x00070072},  // f23
    {0x000000ca, 0x00070073},  // f24
    {0x000000d1, 0x000c00b1},  // mediaPause
    {0x000000d6, 0x000c0203},  // close
    {0x000000d7, 0x000c00b0},  // mediaPlay
    {0x000000d8, 0x000c00b3},  // mediaFastForward
    {0x000000d9, 0x000c00e5},  // bassBoost
    {0x000000da, 0x000c0208},  // print
    {0x000000e1, 0x000c0221},  // browserSearch
    {0x000000e8, 0x000c0070},  // brightnessDown
    {0x000000e9, 0x000c006f},  // brightnessUp
    {0x000000eb, 0x000100b5},  // displayToggleIntExt
    {0x000000ed, 0x000c007a},  // kbdIllumDown
    {0x000000ee, 0x000c0079},  // kbdIllumUp
    {0x000000ef, 0x000c028c},  // mailSend
    {0x000000f0, 0x000c0289},  // mailReply
    {0x000000f1, 0x000c028b},  // mailForward
    {0x000000f2, 0x000c0207},  // save
    {0x000000f3, 0x000c01a7},  // launchDocuments
    {0x000000fc, 0x000c0075},  // brightnessAuto
    {0x00000100, 0x00000018},  // microphoneMuteToggle
    {0x0000016e, 0x000c0060},  // info
    {0x00000172, 0x000c008d},  // programGuide
    {0x0000017a, 0x000c0061},  // closedCaptionToggle
    {0x0000017c, 0x000c0232},  // zoomToggle
    {0x0000017e, 0x000c01ae},  // launchKeyboardLayout
    {0x00000190, 0x000c01b7},  // launchAudioBrowser
    {0x00000195, 0x000c018e},  // launchCalendar
    {0x0000019d, 0x000c0083},  // mediaLast
    {0x000001a2, 0x000c009c},  // channelUp
    {0x000001a3, 0x000c009d},  // channelDown
    {0x000001aa, 0x000c022d},  // zoomIn
    {0x000001ab, 0x000c022e},  // zoomOut
    {0x000001ad, 0x000c0184},  // launchWordProcessor
    {0x000001af, 0x000c0186},  // launchSpreadsheet
    {0x000001b5, 0x000c018d},  // launchContacts
    {0x000001b7, 0x000c0072},  // brightnessToggle
    {0x000001b8, 0x000c01ab},  // spellCheck
    {0x000001b9, 0x000c019c},  // logOff
    {0x0000024b, 0x000c019f},  // launchControlPanel
    {0x0000024c, 0x000c01a2},  // selectTask
    {0x0000024d, 0x000c01b1},  // launchScreenSaver
    {0x0000024e, 0x000c00cf},  // speechInputToggle
    {0x0000024f, 0x000c01cb},  // launchAssistant
    {0x00000250, 0x000c029d},  // keyboardLayoutSelect
    {0x00000258, 0x000c0073},  // brightnessMinimum
    {0x00000259, 0x000c0074},  // brightnessMaximum
    {0x00000281, 0x00000017},  // privacyScreenToggle
};

std::map<uint64_t, uint64_t> gtk_keyval_to_logical_key_map = {
    {0x000000a5, 0x00200000022},  // yen
    {0x0000fd06, 0x00100000405},  // 3270_EraseEOF
    {0x0000fd0e, 0x00100000503},  // 3270_Attn
    {0x0000fd15, 0x00100000402},  // 3270_Copy
    {0x0000fd16, 0x00100000d2f},  // 3270_Play
    {0x0000fd1b, 0x00100000406},  // 3270_ExSelect
    {0x0000fd1d, 0x00100000608},  // 3270_PrintScreen
    {0x0000fd1e, 0x0010000000d},  // 3270_Enter
    {0x0000fe03, 0x00200000105},  // ISO_Level3_Shift
    {0x0000fe08, 0x00100000709},  // ISO_Next_Group
    {0x0000fe0a, 0x0010000070a},  // ISO_Prev_Group
    {0x0000fe0c, 0x00100000707},  // ISO_First_Group
    {0x0000fe0e, 0x00100000708},  // ISO_Last_Group
    {0x0000fe20, 0x00100000009},  // ISO_Left_Tab
    {0x0000fe34, 0x0010000000d},  // ISO_Enter
    {0x0000ff08, 0x00100000008},  // BackSpace
    {0x0000ff09, 0x00100000009},  // Tab
    {0x0000ff0b, 0x00100000401},  // Clear
    {0x0000ff0d, 0x0010000000d},  // Return
    {0x0000ff13, 0x00100000509},  // Pause
    {0x0000ff14, 0x0010000010c},  // Scroll_Lock
    {0x0000ff1b, 0x0010000001b},  // Escape
    {0x0000ff21, 0x00100000719},  // Kanji
    {0x0000ff24, 0x0010000071b},  // Romaji
    {0x0000ff25, 0x00100000716},  // Hiragana
    {0x0000ff26, 0x0010000071a},  // Katakana
    {0x0000ff27, 0x00100000717},  // Hiragana_Katakana
    {0x0000ff28, 0x0010000071c},  // Zenkaku
    {0x0000ff29, 0x00100000715},  // Hankaku
    {0x0000ff2a, 0x0010000071d},  // Zenkaku_Hankaku
    {0x0000ff2f, 0x00100000714},  // Eisu_Shift
    {0x0000ff31, 0x00100000711},  // Hangul
    {0x0000ff34, 0x00100000712},  // Hangul_Hanja
    {0x0000ff37, 0x00100000703},  // Codeinput
    {0x0000ff3c, 0x00100000710},  // SingleCandidate
    {0x0000ff3e, 0x0010000070e},  // PreviousCandidate
    {0x0000ff50, 0x00100000306},  // Home
    {0x0000ff51, 0x00100000302},  // Left
    {0x0000ff52, 0x00100000304},  // Up
    {0x0000ff53, 0x00100000303},  // Right
    {0x0000ff54, 0x00100000301},  // Down
    {0x0000ff55, 0x00100000308},  // Page_Up
    {0x0000ff56, 0x00100000307},  // Page_Down
    {0x0000ff57, 0x00100000305},  // End
    {0x0000ff60, 0x0010000050c},  // Select
    {0x0000ff61, 0x00100000a0c},  // Print
    {0x0000ff62, 0x00100000506},  // Execute
    {0x0000ff63, 0x00100000407},  // Insert
    {0x0000ff65, 0x0010000040a},  // Undo
    {0x0000ff66, 0x00100000409},  // Redo
    {0x0000ff67, 0x00100000505},  // Menu
    {0x0000ff68, 0x00100000507},  // Find
    {0x0000ff69, 0x00100000504},  // Cancel
    {0x0000ff6a, 0x00100000508},  // Help
    {0x0000ff7e, 0x0010000070b},  // Mode_switch
    {0x0000ff7f, 0x0010000010a},  // Num_Lock
    {0x0000ff80, 0x00000000020},  // KP_Space
    {0x0000ff89, 0x00100000009},  // KP_Tab
    {0x0000ff8d, 0x0020000020d},  // KP_Enter
    {0x0000ff91, 0x00100000801},  // KP_F1
    {0x0000ff92, 0x00100000802},  // KP_F2
    {0x0000ff93, 0x00100000803},  // KP_F3
    {0x0000ff94, 0x00100000804},  // KP_F4
    {0x0000ff95, 0x00200000237},  // KP_Home
    {0x0000ff96, 0x00200000234},  // KP_Left
    {0x0000ff97, 0x00200000238},  // KP_Up
    {0x0000ff98, 0x00200000236},  // KP_Right
    {0x0000ff99, 0x00200000232},  // KP_Down
    {0x0000ff9a, 0x00200000239},  // KP_Page_Up
    {0x0000ff9b, 0x00200000233},  // KP_Page_Down
    {0x0000ff9c, 0x00200000231},  // KP_End
    {0x0000ff9e, 0x00200000230},  // KP_Insert
    {0x0000ff9f, 0x0020000022e},  // KP_Delete
    {0x0000ffaa, 0x0020000022a},  // KP_Multiply
    {0x0000ffab, 0x0020000022b},  // KP_Add
    {0x0000ffad, 0x0020000022d},  // KP_Subtract
    {0x0000ffae, 0x0000000002e},  // KP_Decimal
    {0x0000ffaf, 0x0020000022f},  // KP_Divide
    {0x0000ffb0, 0x00200000230},  // KP_0
    {0x0000ffb1, 0x00200000231},  // KP_1
    {0x0000ffb2, 0x00200000232},  // KP_2
    {0x0000ffb3, 0x00200000233},  // KP_3
    {0x0000ffb4, 0x00200000234},  // KP_4
    {0x0000ffb5, 0x00200000235},  // KP_5
    {0x0000ffb6, 0x00200000236},  // KP_6
    {0x0000ffb7, 0x00200000237},  // KP_7
    {0x0000ffb8, 0x00200000238},  // KP_8
    {0x0000ffb9, 0x00200000239},  // KP_9
    {0x0000ffbd, 0x0020000023d},  // KP_Equal
    {0x0000ffbe, 0x00100000801},  // F1
    {0x0000ffbf, 0x00100000802},  // F2
    {0x0000ffc0, 0x00100000803},  // F3
    {0x0000ffc1, 0x00100000804},  // F4
    {0x0000ffc2, 0x00100000805},  // F5
    {0x0000ffc3, 0x00100000806},  // F6
    {0x0000ffc4, 0x00100000807},  // F7
    {0x0000ffc5, 0x00100000808},  // F8
    {0x0000ffc6, 0x00100000809},  // F9
    {0x0000ffc7, 0x0010000080a},  // F10
    {0x0000ffc8, 0x0010000080b},  // F11
    {0x0000ffc9, 0x0010000080c},  // F12
    {0x0000ffca, 0x0010000080d},  // F13
    {0x0000ffcb, 0x0010000080e},  // F14
    {0x0000ffcc, 0x0010000080f},  // F15
    {0x0000ffcd, 0x00100000810},  // F16
    {0x0000ffce, 0x00100000811},  // F17
    {0x0000ffcf, 0x00100000812},  // F18
    {0x0000ffd0, 0x00100000813},  // F19
    {0x0000ffd1, 0x00100000814},  // F20
    {0x0000ffd2, 0x00100000815},  // F21
    {0x0000ffd3, 0x00100000816},  // F22
    {0x0000ffd4, 0x00100000817},  // F23
    {0x0000ffd5, 0x00100000818},  // F24
    {0x0000ffe1, 0x00200000102},  // Shift_L
    {0x0000ffe2, 0x00200000103},  // Shift_R
    {0x0000ffe3, 0x00200000100},  // Control_L
    {0x0000ffe4, 0x00200000101},  // Control_R
    {0x0000ffe5, 0x00100000104},  // Caps_Lock
    {0x0000ffe7, 0x00200000106},  // Meta_L
    {0x0000ffe8, 0x00200000107},  // Meta_R
    {0x0000ffe9, 0x00200000104},  // Alt_L
    {0x0000ffea, 0x00200000105},  // Alt_R
    {0x0000ffeb, 0x0010000010e},  // Super_L
    {0x0000ffec, 0x0010000010e},  // Super_R
    {0x0000ffed, 0x00100000108},  // Hyper_L
    {0x0000ffee, 0x00100000108},  // Hyper_R
    {0x0000ffff, 0x0010000007f},  // Delete
    {0x1008ff02, 0x00100000602},  // MonBrightnessUp
    {0x1008ff03, 0x00100000601},  // MonBrightnessDown
    {0x1008ff10, 0x0010000060a},  // Standby
    {0x1008ff11, 0x00100000a0f},  // AudioLowerVolume
    {0x1008ff12, 0x00100000a11},  // AudioMute
    {0x1008ff13, 0x00100000a10},  // AudioRaiseVolume
    {0x1008ff14, 0x00100000d2f},  // AudioPlay
    {0x1008ff15, 0x00100000a07},  // AudioStop
    {0x1008ff16, 0x00100000a09},  // AudioPrev
    {0x1008ff17, 0x00100000a08},  // AudioNext
    {0x1008ff18, 0x00100000c04},  // HomePage
    {0x1008ff19, 0x00100000b03},  // Mail
    {0x1008ff1b, 0x00100000c06},  // Search
    {0x1008ff1c, 0x00100000d30},  // AudioRecord
    {0x1008ff20, 0x00100000b02},  // Calendar
    {0x1008ff26, 0x00100000c01},  // Back
    {0x1008ff27, 0x00100000c03},  // Forward
    {0x1008ff28, 0x00100000c07},  // Stop
    {0x1008ff29, 0x00100000c05},  // Refresh
    {0x1008ff2a, 0x00100000607},  // PowerOff
    {0x1008ff2b, 0x0010000060b},  // WakeUp
    {0x1008ff2c, 0x00100000604},  // Eject
    {0x1008ff2d, 0x00100000b07},  // ScreenSaver
    {0x1008ff2f, 0x00200000002},  // Sleep
    {0x1008ff30, 0x00100000c02},  // Favorites
    {0x1008ff31, 0x00100000d2e},  // AudioPause
    {0x1008ff3e, 0x00100000d31},  // AudioRewind
    {0x1008ff56, 0x00100000a01},  // Close
    {0x1008ff57, 0x00100000402},  // Copy
    {0x1008ff58, 0x00100000404},  // Cut
    {0x1008ff61, 0x00100000605},  // LogOff
    {0x1008ff68, 0x00100000a0a},  // New
    {0x1008ff6b, 0x00100000a0b},  // Open
    {0x1008ff6d, 0x00100000408},  // Paste
    {0x1008ff6e, 0x00100000b0d},  // Phone
    {0x1008ff72, 0x00100000a03},  // Reply
    {0x1008ff77, 0x00100000a0d},  // Save
    {0x1008ff7b, 0x00100000a04},  // Send
    {0x1008ff7c, 0x00100000a0e},  // Spell
    {0x1008ff8b, 0x0010000050d},  // ZoomIn
    {0x1008ff8c, 0x0010000050e},  // ZoomOut
    {0x1008ff90, 0x00100000a02},  // MailForward
    {0x1008ff97, 0x00100000d2c},  // AudioForward
    {0x1008ffa7, 0x00200000000},  // Suspend
};

void initialize_modifier_bit_to_checked_keys(GHashTable* table) {
  FlKeyEmbedderCheckedKey* data;

  data = g_new(FlKeyEmbedderCheckedKey, 1);
  g_hash_table_insert(table, GUINT_TO_POINTER(GDK_SHIFT_MASK), data);
  data->is_caps_lock = false;
  data->primary_physical_key = 0x0000700e1;     // shiftLeft
  data->primary_logical_key = 0x00200000102;    // shiftLeft
  data->secondary_logical_key = 0x00200000103;  // shiftRight

  data = g_new(FlKeyEmbedderCheckedKey, 1);
  g_hash_table_insert(table, GUINT_TO_POINTER(GDK_CONTROL_MASK), data);
  data->is_caps_lock = false;
  data->primary_physical_key = 0x0000700e0;     // controlLeft
  data->primary_logical_key = 0x00200000100;    // controlLeft
  data->secondary_logical_key = 0x00200000101;  // controlRight

  data = g_new(FlKeyEmbedderCheckedKey, 1);
  g_hash_table_insert(table, GUINT_TO_POINTER(GDK_MOD1_MASK), data);
  data->is_caps_lock = false;
  data->primary_physical_key = 0x0000700e2;     // altLeft
  data->primary_logical_key = 0x00200000104;    // altLeft
  data->secondary_logical_key = 0x00200000105;  // altRight

  data = g_new(FlKeyEmbedderCheckedKey, 1);
  g_hash_table_insert(table, GUINT_TO_POINTER(GDK_META_MASK), data);
  data->is_caps_lock = false;
  data->primary_physical_key = 0x0000700e3;     // metaLeft
  data->primary_logical_key = 0x00200000106;    // metaLeft
  data->secondary_logical_key = 0x00200000107;  // metaRight
}

void initialize_lock_bit_to_checked_keys(GHashTable* table) {
  FlKeyEmbedderCheckedKey* data;

  data = g_new(FlKeyEmbedderCheckedKey, 1);
  g_hash_table_insert(table, GUINT_TO_POINTER(GDK_LOCK_MASK), data);
  data->is_caps_lock = true;
  data->primary_physical_key = 0x000070039;   // capsLock
  data->primary_logical_key = 0x00100000104;  // capsLock

  data = g_new(FlKeyEmbedderCheckedKey, 1);
  g_hash_table_insert(table, GUINT_TO_POINTER(GDK_MOD2_MASK), data);
  data->is_caps_lock = false;
  data->primary_physical_key = 0x000070053;   // numLock
  data->primary_logical_key = 0x0010000010a;  // numLock
}

const std::vector<LayoutGoal> layout_goals = {
    LayoutGoal{0x30, 0x22, false},  // Quote
    LayoutGoal{0x3b, 0x2c, false},  // Comma
    LayoutGoal{0x14, 0x2d, false},  // Minus
    LayoutGoal{0x3c, 0x2e, false},  // Period
    LayoutGoal{0x3d, 0x2f, false},  // Slash
    LayoutGoal{0x13, 0x30, true},   // Digit0
    LayoutGoal{0x0a, 0x31, true},   // Digit1
    LayoutGoal{0x0b, 0x32, true},   // Digit2
    LayoutGoal{0x0c, 0x33, true},   // Digit3
    LayoutGoal{0x0d, 0x34, true},   // Digit4
    LayoutGoal{0x0e, 0x35, true},   // Digit5
    LayoutGoal{0x0f, 0x36, true},   // Digit6
    LayoutGoal{0x10, 0x37, true},   // Digit7
    LayoutGoal{0x11, 0x38, true},   // Digit8
    LayoutGoal{0x12, 0x39, true},   // Digit9
    LayoutGoal{0x2f, 0x3b, false},  // Semicolon
    LayoutGoal{0x15, 0x3d, false},  // Equal
    LayoutGoal{0x22, 0x5b, false},  // BracketLeft
    LayoutGoal{0x33, 0x5c, false},  // Backslash
    LayoutGoal{0x23, 0x5d, false},  // BracketRight
    LayoutGoal{0x31, 0x60, false},  // Backquote
    LayoutGoal{0x26, 0x61, true},   // KeyA
    LayoutGoal{0x38, 0x62, true},   // KeyB
    LayoutGoal{0x36, 0x63, true},   // KeyC
    LayoutGoal{0x28, 0x64, true},   // KeyD
    LayoutGoal{0x1a, 0x65, true},   // KeyE
    LayoutGoal{0x29, 0x66, true},   // KeyF
    LayoutGoal{0x2a, 0x67, true},   // KeyG
    LayoutGoal{0x2b, 0x68, true},   // KeyH
    LayoutGoal{0x1f, 0x69, true},   // KeyI
    LayoutGoal{0x2c, 0x6a, true},   // KeyJ
    LayoutGoal{0x2d, 0x6b, true},   // KeyK
    LayoutGoal{0x2e, 0x6c, true},   // KeyL
    LayoutGoal{0x3a, 0x6d, true},   // KeyM
    LayoutGoal{0x39, 0x6e, true},   // KeyN
    LayoutGoal{0x20, 0x6f, true},   // KeyO
    LayoutGoal{0x21, 0x70, true},   // KeyP
    LayoutGoal{0x18, 0x71, true},   // KeyQ
    LayoutGoal{0x1b, 0x72, true},   // KeyR
    LayoutGoal{0x27, 0x73, true},   // KeyS
    LayoutGoal{0x1c, 0x74, true},   // KeyT
    LayoutGoal{0x1e, 0x75, true},   // KeyU
    LayoutGoal{0x37, 0x76, true},   // KeyV
    LayoutGoal{0x19, 0x77, true},   // KeyW
    LayoutGoal{0x35, 0x78, true},   // KeyX
    LayoutGoal{0x1d, 0x79, true},   // KeyY
    LayoutGoal{0x34, 0x7a, true},   // KeyZ
    LayoutGoal{0x5e, 0x200000020, false},  // IntlBackslash
};

const uint64_t kValueMask = 0x000ffffffff;
const uint64_t kUnicodePlane = 0x00000000000;
const uint64_t kGtkPlane = 0x01500000000;
