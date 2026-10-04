/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --no-compress --no-kerning --font SourceSans3-Regular.ttf --autohint-off -r 0x20-0x7E -r 0xA0-0xFF -r 0x2013-0x2014 -r 0x2018-0x201D -r 0x2026 --size 16 --bpp 1 --format lvgl --lv-font-name latin_ui_16 -o latin_ui_16.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LATIN_UI_16
#define LATIN_UI_16 1
#endif

#if LATIN_UI_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x55, 0x55, 0x3c,

    /* U+0022 "\"" */
    0xde, 0xb5, 0x20,

    /* U+0023 "#" */
    0x24, 0x91, 0x27, 0xf4, 0x89, 0x12, 0x7e, 0x48,
    0x92, 0x40,

    /* U+0024 "$" */
    0x10, 0x47, 0xb1, 0xc3, 0x7, 0x6, 0xc, 0x18,
    0xfe, 0x10, 0x40,

    /* U+0025 "%" */
    0x60, 0x89, 0x10, 0x91, 0x9, 0x20, 0x92, 0xe9,
    0x52, 0x65, 0x10, 0x91, 0x9, 0x11, 0x12, 0x20,
    0xe0,

    /* U+0026 "&" */
    0x38, 0x24, 0x12, 0x9, 0x7, 0x3, 0xb, 0xc5,
    0x34, 0x8e, 0x67, 0xde, 0x40,

    /* U+0027 "'" */
    0xfe,

    /* U+0028 "(" */
    0x25, 0x25, 0xa4, 0x99, 0x24, 0x48,

    /* U+0029 ")" */
    0x91, 0x24, 0x93, 0x69, 0x25, 0x20,

    /* U+002A "*" */
    0x25, 0x5c, 0xa5, 0x0,

    /* U+002B "+" */
    0x30, 0xc3, 0x3f, 0x30, 0xc3, 0x0,

    /* U+002C "," */
    0xf7, 0x80,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x8, 0x42, 0x21, 0x8, 0x84, 0x21, 0x10, 0x84,
    0x40,

    /* U+0030 "0" */
    0x79, 0x28, 0x61, 0x86, 0x18, 0x61, 0x85, 0x27,
    0x80,

    /* U+0031 "1" */
    0x31, 0xc1, 0x4, 0x10, 0x41, 0x4, 0x10, 0x4f,
    0xc0,

    /* U+0032 "2" */
    0x72, 0x20, 0xc3, 0xc, 0x21, 0xc, 0x63, 0xf,
    0xc0,

    /* U+0033 "3" */
    0x3c, 0x88, 0x18, 0x30, 0x43, 0x1, 0x81, 0x3,
    0x8c, 0xf0,

    /* U+0034 "4" */
    0xc, 0xc, 0x14, 0x14, 0x24, 0x64, 0x44, 0xff,
    0x4, 0x4, 0x4,

    /* U+0035 "5" */
    0x3e, 0xc1, 0x83, 0x7, 0xc8, 0xc0, 0x81, 0x3,
    0x8c, 0xf0,

    /* U+0036 "6" */
    0x39, 0x1c, 0x20, 0xbb, 0x38, 0x61, 0x85, 0x33,
    0x80,

    /* U+0037 "7" */
    0xfc, 0x10, 0x86, 0x10, 0x43, 0x8, 0x20, 0x82,
    0x0,

    /* U+0038 "8" */
    0x7b, 0x38, 0x71, 0x69, 0xe8, 0xe1, 0x87, 0x37,
    0x80,

    /* U+0039 "9" */
    0x73, 0x28, 0xe1, 0x86, 0x37, 0x41, 0xe, 0x27,
    0x0,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0xf, 0x78,

    /* U+003C "<" */
    0x4, 0x37, 0x20, 0xe0, 0x70, 0x40,

    /* U+003D "=" */
    0xfc, 0x0, 0x0, 0xfc,

    /* U+003E ">" */
    0x83, 0x3, 0x81, 0x1f, 0x88, 0x0,

    /* U+003F "?" */
    0x74, 0xc2, 0x11, 0x10, 0x84, 0x3, 0x18,

    /* U+0040 "@" */
    0xf, 0x83, 0xc, 0x60, 0x24, 0x72, 0x89, 0x19,
    0x11, 0x91, 0x19, 0x13, 0x93, 0x28, 0xdc, 0x40,
    0x6, 0x8, 0x1f, 0x0,

    /* U+0041 "A" */
    0x18, 0xe, 0x5, 0x6, 0x82, 0x61, 0x31, 0x88,
    0xfe, 0x43, 0x20, 0xb0, 0x60,

    /* U+0042 "B" */
    0xfc, 0xc6, 0xc2, 0xc2, 0xc6, 0xfc, 0xc2, 0xc3,
    0xc3, 0xc2, 0xfc,

    /* U+0043 "C" */
    0x3c, 0x62, 0xc0, 0x80, 0x80, 0x80, 0x80, 0xc0,
    0xc0, 0x63, 0x3c,

    /* U+0044 "D" */
    0xfc, 0xc6, 0xc2, 0xc3, 0xc1, 0xc1, 0xc1, 0xc3,
    0xc2, 0xc6, 0xfc,

    /* U+0045 "E" */
    0xfd, 0x83, 0x6, 0xc, 0x1f, 0xb0, 0x60, 0xc1,
    0x83, 0xf8,

    /* U+0046 "F" */
    0xff, 0xc, 0x30, 0xc3, 0xfc, 0x30, 0xc3, 0xc,
    0x0,

    /* U+0047 "G" */
    0x3e, 0x63, 0xc0, 0x80, 0x80, 0x8f, 0x81, 0x81,
    0xc1, 0x63, 0x3e,

    /* U+0048 "H" */
    0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xff, 0xc1, 0xc1,
    0xc1, 0xc1, 0xc1,

    /* U+0049 "I" */
    0xff, 0xff, 0xfc,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0x84, 0x21, 0xc, 0xdc,

    /* U+004B "K" */
    0xc3, 0xc6, 0xcc, 0xd8, 0xd8, 0xe8, 0xec, 0xc4,
    0xc6, 0xc2, 0xc3,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xf,
    0xc0,

    /* U+004D "M" */
    0xc1, 0xf0, 0xf8, 0x7c, 0x7f, 0x2e, 0x97, 0x53,
    0xb9, 0xcc, 0xe4, 0x70, 0x20,

    /* U+004E "N" */
    0xc1, 0xe1, 0xe1, 0xf1, 0xd1, 0xd9, 0xc9, 0xc5,
    0xc5, 0xc3, 0xc3,

    /* U+004F "O" */
    0x3c, 0x31, 0xb0, 0x50, 0x38, 0x1c, 0xe, 0x7,
    0x3, 0xc1, 0x31, 0x8f, 0x0,

    /* U+0050 "P" */
    0xfd, 0x8f, 0xe, 0x1c, 0x38, 0xff, 0x60, 0xc1,
    0x83, 0x0,

    /* U+0051 "Q" */
    0x3c, 0x31, 0xb0, 0x50, 0x38, 0x1c, 0xe, 0x7,
    0x83, 0x41, 0x33, 0x7, 0x1, 0x80, 0x78,

    /* U+0052 "R" */
    0xfc, 0xc6, 0xc2, 0xc2, 0xc6, 0xfc, 0xc8, 0xcc,
    0xc4, 0xc6, 0xc3,

    /* U+0053 "S" */
    0x3c, 0x8f, 0x6, 0x6, 0x7, 0x83, 0x83, 0x3,
    0x89, 0xe0,

    /* U+0054 "T" */
    0xff, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8,

    /* U+0055 "U" */
    0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
    0x43, 0x62, 0x3c,

    /* U+0056 "V" */
    0xc1, 0xc3, 0x42, 0x42, 0x62, 0x26, 0x24, 0x24,
    0x1c, 0x18, 0x18,

    /* U+0057 "W" */
    0xc2, 0x14, 0x61, 0x47, 0x34, 0x53, 0x65, 0x26,
    0xd2, 0x29, 0x22, 0x9e, 0x28, 0xe2, 0x8c, 0x30,
    0xc0,

    /* U+0058 "X" */
    0xc3, 0x62, 0x24, 0x34, 0x18, 0x18, 0x18, 0x24,
    0x26, 0x42, 0xc3,

    /* U+0059 "Y" */
    0xc3, 0x42, 0x44, 0x24, 0x2c, 0x18, 0x18, 0x10,
    0x10, 0x10, 0x10,

    /* U+005A "Z" */
    0xfe, 0x8, 0x30, 0x41, 0x82, 0x8, 0x30, 0x41,
    0x83, 0xf8,

    /* U+005B "[" */
    0xfb, 0x6d, 0xb6, 0xdb, 0x6d, 0xc0,

    /* U+005C "\\" */
    0x82, 0x10, 0x82, 0x10, 0x84, 0x10, 0x84, 0x10,
    0x84,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x93, 0xc0,

    /* U+005E "^" */
    0x30, 0xc4, 0x92, 0x4a, 0x10,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0xc4, 0x30,

    /* U+0061 "a" */
    0x7a, 0x30, 0x4f, 0xc6, 0x18, 0xdd,

    /* U+0062 "b" */
    0xc1, 0x83, 0x7, 0xec, 0x78, 0x70, 0xe1, 0xc7,
    0x8a, 0xe0,

    /* U+0063 "c" */
    0x3b, 0x18, 0x20, 0x82, 0xc, 0x5e,

    /* U+0064 "d" */
    0x6, 0xc, 0x1b, 0xfc, 0x70, 0xe1, 0xc3, 0x87,
    0x8d, 0xf8,

    /* U+0065 "e" */
    0x3b, 0x18, 0x7f, 0x82, 0xc, 0x4f,

    /* U+0066 "f" */
    0x39, 0x9, 0xf2, 0x10, 0x84, 0x21, 0x8,

    /* U+0067 "g" */
    0x7f, 0x92, 0x16, 0x67, 0x90, 0x20, 0x3e, 0x83,
    0xd, 0xe0,

    /* U+0068 "h" */
    0xc1, 0x83, 0x6, 0xee, 0x58, 0xf1, 0xe3, 0xc7,
    0x8f, 0x18,

    /* U+0069 "i" */
    0xf3, 0xff, 0xfc,

    /* U+006A "j" */
    0x33, 0x3, 0x33, 0x33, 0x33, 0x33, 0x3e,

    /* U+006B "k" */
    0xc1, 0x83, 0x6, 0x2c, 0x9a, 0x3c, 0x7c, 0xc9,
    0x9b, 0x18,

    /* U+006C "l" */
    0xff, 0xff, 0xf4,

    /* U+006D "m" */
    0x99, 0xdc, 0xcf, 0x10, 0xe2, 0x1c, 0x43, 0x88,
    0x71, 0xe, 0x21,

    /* U+006E "n" */
    0x9d, 0xcb, 0x1e, 0x3c, 0x78, 0xf1, 0xe3,

    /* U+006F "o" */
    0x39, 0x8a, 0x1c, 0x18, 0x30, 0xf1, 0x1c,

    /* U+0070 "p" */
    0xbd, 0x8f, 0xe, 0x1c, 0x38, 0xf1, 0x7c, 0xc1,
    0x83, 0x0,

    /* U+0071 "q" */
    0x7f, 0x8e, 0x1c, 0x38, 0x70, 0xf1, 0xbf, 0x6,
    0xc, 0x18,

    /* U+0072 "r" */
    0xbf, 0x31, 0x8c, 0x63, 0x18,

    /* U+0073 "s" */
    0x39, 0x14, 0xc, 0xc, 0x1c, 0x5e,

    /* U+0074 "t" */
    0x21, 0x9, 0xf2, 0x10, 0x84, 0x21, 0xe,

    /* U+0075 "u" */
    0xc7, 0x1c, 0x71, 0xc7, 0x1c, 0xdd,

    /* U+0076 "v" */
    0xc2, 0x8d, 0x13, 0x22, 0x45, 0xe, 0xc,

    /* U+0077 "w" */
    0xc6, 0x28, 0xcd, 0x39, 0x35, 0x22, 0xb4, 0x52,
    0x8e, 0x61, 0x8c,

    /* U+0078 "x" */
    0xc6, 0xc8, 0xa0, 0xc3, 0x85, 0x11, 0x63,

    /* U+0079 "y" */
    0xc2, 0x85, 0x11, 0x22, 0x47, 0x6, 0xc, 0x10,
    0x61, 0x80,

    /* U+007A "z" */
    0x7c, 0x30, 0x84, 0x30, 0x84, 0x3f,

    /* U+007B "{" */
    0x69, 0x24, 0x94, 0x49, 0x24, 0xc0,

    /* U+007C "|" */
    0xff, 0xff,

    /* U+007D "}" */
    0xe2, 0x22, 0x22, 0x21, 0x22, 0x22, 0x2e,

    /* U+007E "~" */
    0xe6, 0x70,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0xf1, 0x55, 0x54,

    /* U+00A2 "¢" */
    0x10, 0x43, 0xdd, 0xd2, 0x49, 0x34, 0x7c, 0x41,
    0x0,

    /* U+00A3 "£" */
    0x39, 0x94, 0x10, 0x43, 0xe2, 0x8, 0x61, 0xf,
    0xc0,

    /* U+00A4 "¤" */
    0xfb, 0x66, 0x42, 0x42, 0x66, 0xff, 0x42,

    /* U+00A5 "¥" */
    0xc3, 0x42, 0x64, 0x24, 0x24, 0x18, 0x7e, 0x18,
    0x7e, 0x18, 0x18,

    /* U+00A6 "¦" */
    0xfe, 0x7f,

    /* U+00A7 "§" */
    0x79, 0x24, 0x18, 0x9a, 0x18, 0x59, 0x18, 0x28,
    0x9e,

    /* U+00A8 "¨" */
    0xd8,

    /* U+00A9 "©" */
    0x1e, 0x18, 0x64, 0xa, 0x31, 0x92, 0x68, 0x1a,
    0x6, 0xc9, 0xde, 0x98, 0x63, 0xe0,

    /* U+00AA "ª" */
    0xe1, 0xf9, 0xf0,

    /* U+00AB "«" */
    0x4a, 0xa9, 0x65, 0x24,

    /* U+00AC "¬" */
    0xfc, 0x10, 0x41,

    /* U+00AD "­" */
    0xe0,

    /* U+00AE "®" */
    0x39, 0x1b, 0xed, 0xad, 0x13, 0x80,

    /* U+00AF "¯" */
    0xf0,

    /* U+00B0 "°" */
    0xe9, 0x9e,

    /* U+00B1 "±" */
    0x30, 0xc3, 0x3f, 0x30, 0xc3, 0x0, 0xfc,

    /* U+00B2 "²" */
    0xeb, 0x12, 0x24, 0xf0,

    /* U+00B3 "³" */
    0x69, 0x16, 0x19, 0xe0,

    /* U+00B4 "´" */
    0x36, 0xc0,

    /* U+00B5 "µ" */
    0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xc4, 0xcc, 0xf3,
    0xc0, 0xc0, 0xc0,

    /* U+00B6 "¶" */
    0x77, 0xdf, 0x7d, 0xf7, 0xd7, 0x41, 0x4, 0x10,
    0x41,

    /* U+00B7 "·" */
    0xf0,

    /* U+00B8 "¸" */
    0x58, 0xe0,

    /* U+00B9 "¹" */
    0xe4, 0x92, 0x40,

    /* U+00BA "º" */
    0x72, 0x62, 0x97, 0x0,

    /* U+00BB "»" */
    0xa2, 0x9a, 0xd5, 0x50,

    /* U+00BC "¼" */
    0x1, 0x98, 0x21, 0x8, 0x21, 0x4, 0x44, 0x89,
    0x92, 0x50, 0x4a, 0x13, 0xe2, 0x8, 0x81, 0x0,

    /* U+00BD "½" */
    0x1, 0x18, 0x21, 0x8, 0x22, 0x4, 0x4c, 0x92,
    0x52, 0x8, 0x82, 0x10, 0x44, 0x10, 0x87, 0x80,

    /* U+00BE "¾" */
    0x60, 0x92, 0x20, 0x44, 0x31, 0x1, 0x25, 0x29,
    0xb9, 0x30, 0x4a, 0x9, 0xe2, 0x8, 0xc1, 0x0,

    /* U+00BF "¿" */
    0x21, 0x80, 0x42, 0x31, 0x10, 0x84, 0x5c,

    /* U+00C0 "À" */
    0x30, 0x6, 0x6, 0x3, 0x81, 0x41, 0xa0, 0x98,
    0x4c, 0x62, 0x3f, 0x90, 0xc8, 0x2c, 0x18,

    /* U+00C1 "Á" */
    0x6, 0x4, 0x6, 0x3, 0x81, 0x41, 0xa0, 0x98,
    0x4c, 0x62, 0x3f, 0x90, 0xc8, 0x2c, 0x18,

    /* U+00C2 "Â" */
    0x1c, 0x13, 0x6, 0x3, 0x81, 0x41, 0xa0, 0x98,
    0x4c, 0x62, 0x3f, 0x90, 0xc8, 0x2c, 0x18,

    /* U+00C3 "Ã" */
    0x32, 0x17, 0x6, 0x3, 0x81, 0x41, 0xa0, 0x98,
    0x4c, 0x62, 0x3f, 0x90, 0xc8, 0x2c, 0x18,

    /* U+00C4 "Ä" */
    0x26, 0x0, 0x6, 0x3, 0x81, 0x41, 0xa0, 0x98,
    0x4c, 0x62, 0x3f, 0x90, 0xc8, 0x2c, 0x18,

    /* U+00C5 "Å" */
    0x1c, 0xa, 0x7, 0x3, 0x1, 0xc0, 0xa0, 0xd0,
    0x4c, 0x26, 0x31, 0x1f, 0xc8, 0x64, 0x16, 0xc,

    /* U+00C6 "Æ" */
    0x7, 0xf0, 0x70, 0xf, 0x0, 0xb0, 0x1b, 0x1,
    0x3e, 0x33, 0x3, 0xf0, 0x63, 0x4, 0x30, 0xc3,
    0xf0,

    /* U+00C7 "Ç" */
    0x3c, 0x62, 0xc0, 0x80, 0x80, 0x80, 0x80, 0xc0,
    0xc0, 0x63, 0x3c, 0x8, 0x18, 0x4, 0x18,

    /* U+00C8 "È" */
    0x60, 0x23, 0xf6, 0xc, 0x18, 0x30, 0x7e, 0xc1,
    0x83, 0x6, 0xf, 0xe0,

    /* U+00C9 "É" */
    0xc, 0x23, 0xf6, 0xc, 0x18, 0x30, 0x7e, 0xc1,
    0x83, 0x6, 0xf, 0xe0,

    /* U+00CA "Ê" */
    0x38, 0xcb, 0xf6, 0xc, 0x18, 0x30, 0x7e, 0xc1,
    0x83, 0x6, 0xf, 0xe0,

    /* U+00CB "Ë" */
    0x6c, 0x3, 0xf6, 0xc, 0x18, 0x30, 0x7e, 0xc1,
    0x83, 0x6, 0xf, 0xe0,

    /* U+00CC "Ì" */
    0xcd, 0xb6, 0xdb, 0x6d, 0xb6,

    /* U+00CD "Í" */
    0x7c, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xc0,

    /* U+00CE "Î" */
    0x33, 0x33, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0x30,

    /* U+00CF "Ï" */
    0x98, 0x18, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xc6,
    0x0,

    /* U+00D0 "Ð" */
    0x3e, 0x11, 0x88, 0x64, 0x12, 0xf, 0xe4, 0x82,
    0x41, 0x21, 0x91, 0x8f, 0x80,

    /* U+00D1 "Ñ" */
    0x32, 0x2e, 0xc1, 0xe1, 0xe1, 0xf1, 0xd1, 0xd9,
    0xc9, 0xc5, 0xc5, 0xc3, 0xc3,

    /* U+00D2 "Ò" */
    0x30, 0x4, 0xf, 0xc, 0x6c, 0x14, 0xe, 0x7,
    0x3, 0x81, 0xc0, 0xf0, 0x4c, 0x63, 0xc0,

    /* U+00D3 "Ó" */
    0x6, 0x4, 0xf, 0xc, 0x6c, 0x14, 0xe, 0x7,
    0x3, 0x81, 0xc0, 0xf0, 0x4c, 0x63, 0xc0,

    /* U+00D4 "Ô" */
    0x18, 0x13, 0x1f, 0xc, 0x6c, 0x14, 0xe, 0x7,
    0x3, 0x81, 0xc0, 0xf0, 0x4c, 0x63, 0xc0,

    /* U+00D5 "Õ" */
    0x32, 0x37, 0xf, 0xc, 0x6c, 0x14, 0xe, 0x7,
    0x3, 0x81, 0xc0, 0xf0, 0x4c, 0x63, 0xc0,

    /* U+00D6 "Ö" */
    0x26, 0x0, 0xf, 0xc, 0x6c, 0x14, 0xe, 0x7,
    0x3, 0x81, 0xc0, 0xf0, 0x4c, 0x63, 0xc0,

    /* U+00D7 "×" */
    0x87, 0x35, 0x8c, 0x31, 0x28, 0x40,

    /* U+00D8 "Ø" */
    0x3f, 0xb1, 0xb0, 0xd8, 0xf8, 0xdc, 0x4e, 0x47,
    0xe3, 0xe1, 0x31, 0xaf, 0x0,

    /* U+00D9 "Ù" */
    0x30, 0x8, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
    0xc1, 0xc1, 0x43, 0x62, 0x3c,

    /* U+00DA "Ú" */
    0x6, 0x18, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
    0xc1, 0xc1, 0x43, 0x62, 0x3c,

    /* U+00DB "Û" */
    0x18, 0x26, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
    0xc1, 0xc1, 0x43, 0x62, 0x3c,

    /* U+00DC "Ü" */
    0x26, 0x0, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
    0xc1, 0xc1, 0x43, 0x62, 0x3c,

    /* U+00DD "Ý" */
    0xc, 0x10, 0xc3, 0x42, 0x44, 0x24, 0x2c, 0x18,
    0x18, 0x10, 0x10, 0x10, 0x10,

    /* U+00DE "Þ" */
    0xc0, 0xc0, 0xfc, 0xc6, 0xc3, 0xc3, 0xc2, 0xfc,
    0xc0, 0xc0, 0xc0,

    /* U+00DF "ß" */
    0x38, 0x44, 0xc4, 0xc4, 0xc8, 0xc8, 0xcc, 0xc2,
    0xc3, 0xd3, 0xde,

    /* U+00E0 "à" */
    0x40, 0x81, 0x2, 0x7a, 0x30, 0x4f, 0xc6, 0x18,
    0xdd,

    /* U+00E1 "á" */
    0xc, 0x63, 0x0, 0x7a, 0x30, 0x4f, 0xc6, 0x18,
    0xdd,

    /* U+00E2 "â" */
    0x10, 0xe4, 0xc0, 0x7a, 0x30, 0x4f, 0xc6, 0x18,
    0xdd,

    /* U+00E3 "ã" */
    0x65, 0x78, 0x1e, 0x8c, 0x13, 0xf1, 0x86, 0x37,
    0x40,

    /* U+00E4 "ä" */
    0x6c, 0x0, 0x1e, 0x8c, 0x13, 0xf1, 0x86, 0x37,
    0x40,

    /* U+00E5 "å" */
    0x31, 0x24, 0x8c, 0x1, 0xe8, 0xc1, 0x3f, 0x18,
    0x63, 0x74,

    /* U+00E6 "æ" */
    0x7b, 0xd1, 0xc8, 0x10, 0xbf, 0xfc, 0x41, 0x8,
    0x23, 0x93, 0x9f,

    /* U+00E7 "ç" */
    0x3b, 0x18, 0x20, 0x82, 0xc, 0x5e, 0x10, 0xc1,
    0x8c,

    /* U+00E8 "è" */
    0x40, 0x81, 0x0, 0x3b, 0x18, 0x7f, 0x82, 0xc,
    0x4f,

    /* U+00E9 "é" */
    0xc, 0x63, 0x0, 0x3b, 0x18, 0x7f, 0x82, 0xc,
    0x4f,

    /* U+00EA "ê" */
    0x10, 0xec, 0xc0, 0x3b, 0x18, 0x7f, 0x82, 0xc,
    0x4f,

    /* U+00EB "ë" */
    0x4c, 0x0, 0xe, 0xc6, 0x1f, 0xe0, 0x83, 0x13,
    0xc0,

    /* U+00EC "ì" */
    0x88, 0x86, 0xdb, 0x6d, 0xb0,

    /* U+00ED "í" */
    0x2a, 0xd, 0xb6, 0xdb, 0x60,

    /* U+00EE "î" */
    0x30, 0xcc, 0xc0, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc,

    /* U+00EF "ï" */
    0x90, 0x6, 0x66, 0x66, 0x66, 0x60,

    /* U+00F0 "ð" */
    0x40, 0xd8, 0xe3, 0x40, 0x4f, 0xb1, 0xc3, 0x87,
    0xf, 0x13, 0xc0,

    /* U+00F1 "ñ" */
    0x74, 0xb8, 0x4, 0xee, 0x58, 0xf1, 0xe3, 0xc7,
    0x8f, 0x18,

    /* U+00F2 "ò" */
    0x60, 0x40, 0x60, 0x3, 0x98, 0xa1, 0xc1, 0x83,
    0xf, 0x11, 0xc0,

    /* U+00F3 "ó" */
    0xc, 0x30, 0xc0, 0x3, 0x98, 0xa1, 0xc1, 0x83,
    0xf, 0x11, 0xc0,

    /* U+00F4 "ô" */
    0x10, 0x71, 0x30, 0x3, 0x98, 0xa1, 0xc1, 0x83,
    0xf, 0x11, 0xc0,

    /* U+00F5 "õ" */
    0x64, 0xb8, 0x1, 0xcc, 0x50, 0xe0, 0xc1, 0x87,
    0x88, 0xe0,

    /* U+00F6 "ö" */
    0x6c, 0x0, 0x1, 0xcc, 0x50, 0xe0, 0xc1, 0x87,
    0x88, 0xe0,

    /* U+00F7 "÷" */
    0x30, 0xc0, 0x0, 0xfc, 0x3, 0xc,

    /* U+00F8 "ø" */
    0x3b, 0x8a, 0x3c, 0x9b, 0x3c, 0xf1, 0x5c,

    /* U+00F9 "ù" */
    0x60, 0x81, 0x80, 0xc7, 0x1c, 0x71, 0xc7, 0x1c,
    0xdd,

    /* U+00FA "ú" */
    0xc, 0x63, 0x0, 0xc7, 0x1c, 0x71, 0xc7, 0x1c,
    0xdd,

    /* U+00FB "û" */
    0x10, 0xe4, 0xc0, 0xc7, 0x1c, 0x71, 0xc7, 0x1c,
    0xdd,

    /* U+00FC "ü" */
    0x6c, 0x0, 0x31, 0xc7, 0x1c, 0x71, 0xc7, 0x37,
    0x40,

    /* U+00FD "ý" */
    0x4, 0x10, 0x40, 0xc, 0x28, 0x51, 0x12, 0x24,
    0x70, 0x60, 0xc1, 0x6, 0x18, 0x0,

    /* U+00FE "þ" */
    0xc1, 0x83, 0x7, 0xec, 0x78, 0x70, 0xe1, 0xc7,
    0x8b, 0xe6, 0xc, 0x18, 0x0,

    /* U+00FF "ÿ" */
    0x24, 0x0, 0x6, 0x14, 0x28, 0x89, 0x12, 0x38,
    0x30, 0x60, 0x83, 0xc, 0x0,

    /* U+2013 "–" */
    0xfc,

    /* U+2014 "—" */
    0xff, 0xe0,

    /* U+2018 "‘" */
    0x6f,

    /* U+2019 "’" */
    0xd6,

    /* U+201A "‚" */
    0xd6,

    /* U+201B "‛" */
    0xe9,

    /* U+201C "“" */
    0x5c, 0xb5, 0xb0,

    /* U+201D "”" */
    0xda, 0x53, 0x20,

    /* U+2026 "…" */
    0x86, 0x38, 0x63
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 51, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 74, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 109, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 7, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 127, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 28, .adv_w = 211, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 156, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 59, .adv_w = 78, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 65, .adv_w = 78, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 71, .adv_w = 107, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 75, .adv_w = 127, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 81, .adv_w = 64, .box_w = 2, .box_h = 5, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 83, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 84, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 90, .box_w = 5, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 94, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 131, .adv_w = 127, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 64, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 64, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 193, .adv_w = 127, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 199, .adv_w = 127, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 203, .adv_w = 127, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 209, .adv_w = 109, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 217, .box_w = 12, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 236, .adv_w = 139, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 151, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 146, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 157, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 135, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 158, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 167, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 67, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 123, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 148, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 124, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 186, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 366, .adv_w = 166, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 170, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 145, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 400, .adv_w = 170, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 415, .adv_w = 146, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 165, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 132, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 201, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 131, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 122, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 508, .adv_w = 138, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 78, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 524, .adv_w = 90, .box_w = 5, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 533, .adv_w = 78, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 539, .adv_w = 127, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 544, .adv_w = 128, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 545, .adv_w = 139, .box_w = 4, .box_h = 3, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 547, .adv_w = 129, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 569, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 579, .adv_w = 127, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 585, .adv_w = 75, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 592, .adv_w = 129, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 602, .adv_w = 139, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 63, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 615, .adv_w = 63, .box_w = 4, .box_h = 14, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 622, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 632, .adv_w = 65, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 635, .adv_w = 212, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 140, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 653, .adv_w = 139, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 660, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 670, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 680, .adv_w = 89, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 685, .adv_w = 107, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 691, .adv_w = 87, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 139, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 704, .adv_w = 120, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 711, .adv_w = 184, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 722, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 729, .adv_w = 120, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 739, .adv_w = 109, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 78, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 751, .adv_w = 62, .box_w = 1, .box_h = 16, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 753, .adv_w = 78, .box_w = 4, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 760, .adv_w = 127, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 762, .adv_w = 51, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 763, .adv_w = 74, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 766, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 775, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 784, .adv_w = 127, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 791, .adv_w = 127, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 802, .adv_w = 62, .box_w = 1, .box_h = 16, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 804, .adv_w = 127, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 813, .adv_w = 139, .box_w = 5, .box_h = 1, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 814, .adv_w = 190, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 828, .adv_w = 88, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 831, .adv_w = 110, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 835, .adv_w = 127, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 838, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 839, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 845, .adv_w = 139, .box_w = 4, .box_h = 1, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 846, .adv_w = 85, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 848, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 855, .adv_w = 94, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 859, .adv_w = 94, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 863, .adv_w = 139, .box_w = 4, .box_h = 3, .ofs_x = 3, .ofs_y = 9},
    {.bitmap_index = 865, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 876, .adv_w = 143, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 885, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 886, .adv_w = 139, .box_w = 3, .box_h = 4, .ofs_x = 3, .ofs_y = -4},
    {.bitmap_index = 888, .adv_w = 94, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 891, .adv_w = 93, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 895, .adv_w = 110, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 899, .adv_w = 200, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 915, .adv_w = 207, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 931, .adv_w = 204, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 947, .adv_w = 109, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 954, .adv_w = 139, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 969, .adv_w = 139, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 984, .adv_w = 139, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 999, .adv_w = 139, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1014, .adv_w = 139, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1029, .adv_w = 139, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1045, .adv_w = 210, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1062, .adv_w = 146, .box_w = 8, .box_h = 15, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 1077, .adv_w = 135, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1089, .adv_w = 135, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1101, .adv_w = 135, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1113, .adv_w = 135, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1125, .adv_w = 67, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1130, .adv_w = 67, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1137, .adv_w = 67, .box_w = 6, .box_h = 13, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1147, .adv_w = 67, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1156, .adv_w = 164, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1169, .adv_w = 166, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1182, .adv_w = 170, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1197, .adv_w = 170, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1212, .adv_w = 170, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1227, .adv_w = 170, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1242, .adv_w = 170, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1257, .adv_w = 127, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 1263, .adv_w = 170, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1276, .adv_w = 165, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1289, .adv_w = 165, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1302, .adv_w = 165, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1315, .adv_w = 165, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1328, .adv_w = 122, .box_w = 8, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1341, .adv_w = 149, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1352, .adv_w = 147, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1363, .adv_w = 129, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1372, .adv_w = 129, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1381, .adv_w = 129, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1390, .adv_w = 129, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1399, .adv_w = 129, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1408, .adv_w = 129, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1418, .adv_w = 199, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1429, .adv_w = 117, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 1438, .adv_w = 127, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1447, .adv_w = 127, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1456, .adv_w = 127, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1465, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1474, .adv_w = 63, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1479, .adv_w = 63, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1484, .adv_w = 63, .box_w = 6, .box_h = 12, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1493, .adv_w = 63, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1499, .adv_w = 139, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1510, .adv_w = 140, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1520, .adv_w = 139, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1531, .adv_w = 139, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1542, .adv_w = 139, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1553, .adv_w = 139, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1563, .adv_w = 139, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1573, .adv_w = 127, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 1579, .adv_w = 139, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1586, .adv_w = 139, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1595, .adv_w = 139, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1604, .adv_w = 139, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1613, .adv_w = 139, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1622, .adv_w = 120, .box_w = 7, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1636, .adv_w = 142, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1649, .adv_w = 120, .box_w = 7, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1662, .adv_w = 123, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 1663, .adv_w = 205, .box_w = 11, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 1665, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 1666, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 1667, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1668, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 1669, .adv_w = 109, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 1672, .adv_w = 109, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 1675, .adv_w = 243, .box_w = 12, .box_h = 2, .ofs_x = 2, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_2[] = {
    0x0, 0x1, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
    0x13
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 160, .range_length = 96, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 8211, .range_length = 20, .glyph_id_start = 192,
        .unicode_list = unicode_list_2, .glyph_id_ofs_list = NULL, .list_length = 9, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 3,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t latin_ui_16 = {
#else
lv_font_t latin_ui_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LATIN_UI_16*/

